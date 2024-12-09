	int err = 0;
	u8 *smac_v;

	if (vport->info.spoofchk && !is_valid_ether_addr(vport->info.mac)) {
		mlx5_core_warn(esw->dev,
			       "vport[%d] configure ingress rules failed, illegal mac with spoofchk\n",
			       vport->vport);
		return -EPERM;
	}

	esw_vport_cleanup_ingress_rules(esw, vport);

	if (!vport->info.vlan && !vport->info.qos && !vport->info.spoofchk) {
		esw_vport_disable_ingress_acl(esw, vport);
	int vport_num;
	int err;

	if (!MLX5_ESWITCH_MANAGER(dev))
		return 0;

	esw_info(dev,
		 "Total vports %d, per vport: max uc(%d) max mc(%d)\n",

void mlx5_eswitch_cleanup(struct mlx5_eswitch *esw)
{
	if (!esw || !MLX5_ESWITCH_MANAGER(esw->dev))
		return;

	esw_info(esw->dev, "cleanup\n");

	mutex_lock(&esw->state_lock);
	evport = &esw->vports[vport];

	if (evport->info.spoofchk && !is_valid_ether_addr(mac)) {
		mlx5_core_warn(esw->dev,
			       "MAC invalidation is not allowed when spoofchk is on, vport(%d)\n",
			       vport);
		err = -EPERM;
		goto unlock;
	}

	err = mlx5_modify_nic_vport_mac_address(esw->dev, vport, mac);
	if (err) {
		mlx5_core_warn(esw->dev,
	evport = &esw->vports[vport];
	pschk = evport->info.spoofchk;
	evport->info.spoofchk = spoofchk;
	if (evport->enabled && esw->mode == SRIOV_LEGACY)
		err = esw_vport_ingress_config(esw, evport);
	if (err)
		evport->info.spoofchk = pschk;