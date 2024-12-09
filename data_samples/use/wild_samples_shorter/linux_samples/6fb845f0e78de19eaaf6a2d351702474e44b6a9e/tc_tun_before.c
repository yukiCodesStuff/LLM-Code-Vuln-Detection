	e->m_neigh.family = n->ops->family;
	memcpy(&e->m_neigh.dst_ip, n->primary_key, n->tbl->key_len);
	e->out_dev = out_dev;

	/* It's important to add the neigh to the hash table before checking
	 * the neigh validity state. So if we'll get a notification, in case the
	 * neigh changes it's validity state, we would find the relevant neigh
	e->m_neigh.family = n->ops->family;
	memcpy(&e->m_neigh.dst_ip, n->primary_key, n->tbl->key_len);
	e->out_dev = out_dev;

	/* It's importent to add the neigh to the hash table before checking
	 * the neigh validity state. So if we'll get a notification, in case the
	 * neigh changes it's validity state, we would find the relevant neigh
		       struct mlx5_flow_spec *spec,
		       struct tc_cls_flower_offload *f,
		       void *headers_c,
		       void *headers_v)
{
	int tunnel_type;
	int err = 0;

	tunnel_type = mlx5e_tc_tun_get_type(filter_dev);
	if (tunnel_type == MLX5E_TC_TUNNEL_TYPE_VXLAN) {
		err = mlx5e_tc_tun_parse_vxlan(priv, spec, f,
					       headers_c, headers_v);
	} else if (tunnel_type == MLX5E_TC_TUNNEL_TYPE_GRETAP) {
		err = mlx5e_tc_tun_parse_gretap(priv, spec, f,
						headers_c, headers_v);
	} else {
		netdev_warn(priv->netdev,