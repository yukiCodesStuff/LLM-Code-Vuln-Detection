	struct net_device *filter_dev;
	struct mlx5_flow_spec spec;
	int num_mod_hdr_actions;
	void *mod_hdr_actions;
	int mirred_ifindex[MLX5_MAX_FLOW_FWD_VPORTS];
};

static int parse_tunnel_attr(struct mlx5e_priv *priv,
			     struct mlx5_flow_spec *spec,
			     struct tc_cls_flower_offload *f,
			     struct net_device *filter_dev)
{
	struct netlink_ext_ack *extack = f->common.extack;
	void *headers_c = MLX5_ADDR_OF(fte_match_param, spec->match_criteria,
				       outer_headers);
	int err = 0;

	err = mlx5e_tc_tun_parse(filter_dev, priv, spec, f,
				 headers_c, headers_v);
	if (err) {
		NL_SET_ERR_MSG_MOD(extack,
				   "failed to parse tunnel attributes");
		return err;
			      struct mlx5_flow_spec *spec,
			      struct tc_cls_flower_offload *f,
			      struct net_device *filter_dev,
			      u8 *match_level)
{
	struct netlink_ext_ack *extack = f->common.extack;
	void *headers_c = MLX5_ADDR_OF(fte_match_param, spec->match_criteria,
				       outer_headers);
		switch (key->addr_type) {
		case FLOW_DISSECTOR_KEY_IPV4_ADDRS:
		case FLOW_DISSECTOR_KEY_IPV6_ADDRS:
			if (parse_tunnel_attr(priv, spec, f, filter_dev))
				return -EOPNOTSUPP;
			break;
		default:
			return -EOPNOTSUPP;
	struct mlx5_core_dev *dev = priv->mdev;
	struct mlx5_eswitch *esw = dev->priv.eswitch;
	struct mlx5e_rep_priv *rpriv = priv->ppriv;
	struct mlx5_eswitch_rep *rep;
	u8 match_level;
	int err;

	err = __parse_cls_flower(priv, spec, f, filter_dev, &match_level);

	if (!err && (flow->flags & MLX5E_TC_FLOW_ESWITCH)) {
		rep = rpriv->rep;
		if (rep->vport != FDB_UPLINK_VPORT &&
		}
	}

	if (flow->flags & MLX5E_TC_FLOW_ESWITCH)
		flow->esw_attr->match_level = match_level;
	else
		flow->nic_attr->match_level = match_level;

	return err;
}

	OFFLOAD(UDP_DPORT, 2, udp.dest,   0),
};

/* On input attr->num_mod_hdr_actions tells how many HW actions can be parsed at
 * max from the SW pedit action. On success, it says how many HW actions were
 * actually parsed.
 */
static int offload_pedit_fields(struct pedit_headers *masks,
				struct pedit_headers *vals,
				struct mlx5e_tc_flow_parse_attr *parse_attr,
	add_vals = &vals[TCA_PEDIT_KEY_EX_CMD_ADD];

	action_size = MLX5_UN_SZ_BYTES(set_action_in_add_action_in_auto);
	action = parse_attr->mod_hdr_actions;
	max_actions = parse_attr->num_mod_hdr_actions;
	nactions = 0;

	for (i = 0; i < ARRAY_SIZE(fields); i++) {
		f = &fields[i];
		/* avoid seeing bits set from previous iterations */
	if (!parse_attr->mod_hdr_actions)
		return -ENOMEM;

	parse_attr->num_mod_hdr_actions = max_actions;
	return 0;
}

static const struct pedit_headers zero_masks = {};
			goto out_err;
	}

	err = alloc_mod_hdr_actions(priv, a, namespace, parse_attr);
	if (err)
		goto out_err;

	err = offload_pedit_fields(masks, vals, parse_attr, extack);
	if (err < 0)
		goto out_dealloc_parsed_actions;

static bool modify_header_match_supported(struct mlx5_flow_spec *spec,
					  struct tcf_exts *exts,
					  struct netlink_ext_ack *extack)
{
	const struct tc_action *a;
	bool modify_ip_header;
	u16 ethertype;
	int nkeys, i;

	headers_v = MLX5_ADDR_OF(fte_match_param, spec->match_value, outer_headers);
	ethertype = MLX5_GET(fte_match_set_lyr_2_4, headers_v, ethertype);

	/* for non-IP we only re-write MACs, so we're okay */
	if (ethertype != ETH_P_IP && ethertype != ETH_P_IPV6)

	if (actions & MLX5_FLOW_CONTEXT_ACTION_MOD_HDR)
		return modify_header_match_supported(&parse_attr->spec, exts,
						     extack);

	return true;
}
