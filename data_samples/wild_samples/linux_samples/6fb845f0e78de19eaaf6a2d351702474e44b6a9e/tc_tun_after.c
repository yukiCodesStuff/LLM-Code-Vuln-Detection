	if (max_encap_size < ipv4_encap_size) {
		mlx5_core_warn(priv->mdev, "encap size %d too big, max supported is %d\n",
			       ipv4_encap_size, max_encap_size);
		return -EOPNOTSUPP;
	}

	encap_header = kzalloc(ipv4_encap_size, GFP_KERNEL);
	if (!encap_header)
		return -ENOMEM;

	/* used by mlx5e_detach_encap to lookup a neigh hash table
	 * entry in the neigh hash table when a user deletes a rule
	 */
	e->m_neigh.dev = n->dev;
	e->m_neigh.family = n->ops->family;
	memcpy(&e->m_neigh.dst_ip, n->primary_key, n->tbl->key_len);
	e->out_dev = out_dev;
	e->route_dev = route_dev;

	/* It's important to add the neigh to the hash table before checking
	 * the neigh validity state. So if we'll get a notification, in case the
	 * neigh changes it's validity state, we would find the relevant neigh
	 * in the hash.
	 */
	err = mlx5e_rep_encap_entry_attach(netdev_priv(out_dev), e);
	if (err)
		goto free_encap;

	read_lock_bh(&n->lock);
	nud_state = n->nud_state;
	ether_addr_copy(e->h_dest, n->ha);
	read_unlock_bh(&n->lock);

	/* add ethernet header */
	ip = (struct iphdr *)gen_eth_tnl_hdr(encap_header, route_dev, e,
					     ETH_P_IP);

	/* add ip header */
	ip->tos = tun_key->tos;
	ip->version = 0x4;
	ip->ihl = 0x5;
	ip->ttl = ttl;
	ip->daddr = fl4.daddr;
	ip->saddr = fl4.saddr;

	/* add tunneling protocol header */
	err = mlx5e_gen_ip_tunnel_header((char *)ip + sizeof(struct iphdr),
					 &ip->protocol, e);
	if (err)
		goto destroy_neigh_entry;

	e->encap_size = ipv4_encap_size;
	e->encap_header = encap_header;

	if (!(nud_state & NUD_VALID)) {
		neigh_event_send(n, NULL);
		err = -EAGAIN;
		goto out;
	}
	if (max_encap_size < ipv6_encap_size) {
		mlx5_core_warn(priv->mdev, "encap size %d too big, max supported is %d\n",
			       ipv6_encap_size, max_encap_size);
		return -EOPNOTSUPP;
	}

	encap_header = kzalloc(ipv6_encap_size, GFP_KERNEL);
	if (!encap_header)
		return -ENOMEM;

	/* used by mlx5e_detach_encap to lookup a neigh hash table
	 * entry in the neigh hash table when a user deletes a rule
	 */
	e->m_neigh.dev = n->dev;
	e->m_neigh.family = n->ops->family;
	memcpy(&e->m_neigh.dst_ip, n->primary_key, n->tbl->key_len);
	e->out_dev = out_dev;
	e->route_dev = route_dev;

	/* It's importent to add the neigh to the hash table before checking
	 * the neigh validity state. So if we'll get a notification, in case the
	 * neigh changes it's validity state, we would find the relevant neigh
	 * in the hash.
	 */
	err = mlx5e_rep_encap_entry_attach(netdev_priv(out_dev), e);
	if (err)
		goto free_encap;

	read_lock_bh(&n->lock);
	nud_state = n->nud_state;
	ether_addr_copy(e->h_dest, n->ha);
	read_unlock_bh(&n->lock);

	/* add ethernet header */
	ip6h = (struct ipv6hdr *)gen_eth_tnl_hdr(encap_header, route_dev, e,
						 ETH_P_IPV6);

	/* add ip header */
	ip6_flow_hdr(ip6h, tun_key->tos, 0);
	/* the HW fills up ipv6 payload len */
	ip6h->hop_limit   = ttl;
	ip6h->daddr	  = fl6.daddr;
	ip6h->saddr	  = fl6.saddr;

	/* add tunneling protocol header */
	err = mlx5e_gen_ip_tunnel_header((char *)ip6h + sizeof(struct ipv6hdr),
					 &ip6h->nexthdr, e);
	if (err)
		goto destroy_neigh_entry;

	e->encap_size = ipv6_encap_size;
	e->encap_header = encap_header;

	if (!(nud_state & NUD_VALID)) {
		neigh_event_send(n, NULL);
		err = -EAGAIN;
		goto out;
	}
	if (dissector_uses_key(f->dissector, FLOW_DISSECTOR_KEY_ENC_KEYID)) {
		struct flow_dissector_key_keyid *mask = NULL;
		struct flow_dissector_key_keyid *key = NULL;

		mask = skb_flow_dissector_target(f->dissector,
						 FLOW_DISSECTOR_KEY_ENC_KEYID,
						 f->mask);
		MLX5_SET(fte_match_set_misc, misc_c,
			 gre_key.key, be32_to_cpu(mask->keyid));

		key = skb_flow_dissector_target(f->dissector,
						FLOW_DISSECTOR_KEY_ENC_KEYID,
						f->key);
		MLX5_SET(fte_match_set_misc, misc_v,
			 gre_key.key, be32_to_cpu(key->keyid));
	}

	return 0;
}

int mlx5e_tc_tun_parse(struct net_device *filter_dev,
		       struct mlx5e_priv *priv,
		       struct mlx5_flow_spec *spec,
		       struct tc_cls_flower_offload *f,
		       void *headers_c,
		       void *headers_v, u8 *match_level)
{
	int tunnel_type;
	int err = 0;

	tunnel_type = mlx5e_tc_tun_get_type(filter_dev);
	if (tunnel_type == MLX5E_TC_TUNNEL_TYPE_VXLAN) {
		*match_level = MLX5_MATCH_L4;
		err = mlx5e_tc_tun_parse_vxlan(priv, spec, f,
					       headers_c, headers_v);
	} else if (tunnel_type == MLX5E_TC_TUNNEL_TYPE_GRETAP) {
		*match_level = MLX5_MATCH_L3;
		err = mlx5e_tc_tun_parse_gretap(priv, spec, f,
						headers_c, headers_v);
	} else {
		netdev_warn(priv->netdev,
			    "decapsulation offload is not supported for %s net device (%d)\n",
			    mlx5e_netdev_kind(filter_dev), tunnel_type);
		return -EOPNOTSUPP;
	}
	return err;
}