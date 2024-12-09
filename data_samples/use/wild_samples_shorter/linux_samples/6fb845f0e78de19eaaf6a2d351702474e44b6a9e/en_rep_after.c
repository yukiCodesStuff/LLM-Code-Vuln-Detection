	if (neigh_connected && !(e->flags & MLX5_ENCAP_ENTRY_VALID)) {
		ether_addr_copy(e->h_dest, ha);
		ether_addr_copy(eth->h_dest, ha);
		/* Update the encap source mac, in case that we delete
		 * the flows when encap source mac changed.
		 */
		ether_addr_copy(eth->h_source, e->route_dev->dev_addr);

		mlx5e_tc_encap_flows_add(priv, e);
	}
}
	struct mlx5e_priv *priv = netdev_priv(dev);
	struct mlx5e_rep_priv *rpriv = priv->ppriv;
	struct mlx5_eswitch_rep *rep = rpriv->rep;
	int ret, pf_num;

	ret = mlx5_lag_get_pf_num(priv->mdev, &pf_num);
	if (ret)
		return ret;

	if (rep->vport == FDB_UPLINK_VPORT)
		ret = snprintf(buf, len, "p%d", pf_num);
	else
		ret = snprintf(buf, len, "pf%dvf%d", pf_num, rep->vport - 1);

	if (ret >= len)
		return -EOPNOTSUPP;

	return 0;
	return 0;
}

static int mlx5e_uplink_rep_set_vf_vlan(struct net_device *dev, int vf, u16 vlan, u8 qos,
					__be16 vlan_proto)
{
	netdev_warn_once(dev, "legacy vf vlan setting isn't supported in switchdev mode\n");

	if (vlan != 0)
		return -EOPNOTSUPP;

	/* allow setting 0-vid for compatibility with libvirt */
	return 0;
}

static const struct switchdev_ops mlx5e_rep_switchdev_ops = {
	.switchdev_port_attr_get	= mlx5e_attr_get,
};

	.ndo_set_vf_rate         = mlx5e_set_vf_rate,
	.ndo_get_vf_config       = mlx5e_get_vf_config,
	.ndo_get_vf_stats        = mlx5e_get_vf_stats,
	.ndo_set_vf_vlan         = mlx5e_uplink_rep_set_vf_vlan,
};

bool mlx5e_eswitch_rep(struct net_device *netdev)
{