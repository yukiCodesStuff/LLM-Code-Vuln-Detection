{
	struct mlx5_fc *counter = vport->ingress.drop_counter;
	struct mlx5_flow_destination drop_ctr_dst = {0};
	struct mlx5_flow_destination *dst = NULL;
	struct mlx5_flow_act flow_act = {0};
	struct mlx5_flow_spec *spec;
	int dest_num = 0;
	int err = 0;
	u8 *smac_v;

	if (vport->info.spoofchk && !is_valid_ether_addr(vport->info.mac)) {
		mlx5_core_warn(esw->dev,
			       "vport[%d] configure ingress rules failed, illegal mac with spoofchk\n",
			       vport->vport);
		return -EPERM;
	}
{
	int total_vports = MLX5_TOTAL_VPORTS(dev);
	struct mlx5_eswitch *esw;
	int vport_num;
	int err;

	if (!MLX5_ESWITCH_MANAGER(dev))
		return 0;

	esw_info(dev,
		 "Total vports %d, per vport: max uc(%d) max mc(%d)\n",
		 total_vports,
		 MLX5_MAX_UC_PER_VPORT(dev),
		 MLX5_MAX_MC_PER_VPORT(dev));

	esw = kzalloc(sizeof(*esw), GFP_KERNEL);
	if (!esw)
		return -ENOMEM;

	esw->dev = dev;

	esw->work_queue = create_singlethread_workqueue("mlx5_esw_wq");
	if (!esw->work_queue) {
		err = -ENOMEM;
		goto abort;
	}
	for (vport_num = 0; vport_num < total_vports; vport_num++) {
		struct mlx5_vport *vport = &esw->vports[vport_num];

		vport->vport = vport_num;
		vport->info.link_state = MLX5_VPORT_ADMIN_STATE_AUTO;
		vport->dev = dev;
		INIT_WORK(&vport->vport_change_handler,
			  esw_vport_change_handler);
	}

	esw->total_vports = total_vports;
	esw->enabled_vports = 0;
	esw->mode = SRIOV_NONE;
	esw->offloads.inline_mode = MLX5_INLINE_MODE_NONE;
	if (MLX5_CAP_ESW_FLOWTABLE_FDB(dev, reformat) &&
	    MLX5_CAP_ESW_FLOWTABLE_FDB(dev, decap))
		esw->offloads.encap = DEVLINK_ESWITCH_ENCAP_MODE_BASIC;
	else
		esw->offloads.encap = DEVLINK_ESWITCH_ENCAP_MODE_NONE;

	dev->priv.eswitch = esw;
	return 0;
abort:
	if (esw->work_queue)
		destroy_workqueue(esw->work_queue);
	esw_offloads_cleanup_reps(esw);
	kfree(esw->vports);
	kfree(esw);
	return err;
}

void mlx5_eswitch_cleanup(struct mlx5_eswitch *esw)
{
	if (!esw || !MLX5_ESWITCH_MANAGER(esw->dev))
		return;

	esw_info(esw->dev, "cleanup\n");

	esw->dev->priv.eswitch = NULL;
	destroy_workqueue(esw->work_queue);
	esw_offloads_cleanup_reps(esw);
	kfree(esw->vports);
	kfree(esw);
}
{
	struct mlx5_vport *evport;
	u64 node_guid;
	int err = 0;

	if (!MLX5_CAP_GEN(esw->dev, vport_group_manager))
		return -EPERM;
	if (!LEGAL_VPORT(esw, vport) || is_multicast_ether_addr(mac))
		return -EINVAL;

	mutex_lock(&esw->state_lock);
	evport = &esw->vports[vport];

	if (evport->info.spoofchk && !is_valid_ether_addr(mac)) {
		mlx5_core_warn(esw->dev,
			       "MAC invalidation is not allowed when spoofchk is on, vport(%d)\n",
			       vport);
		err = -EPERM;
		goto unlock;
	}
{
	struct mlx5_vport *evport;
	bool pschk;
	int err = 0;

	if (!ESW_ALLOWED(esw))
		return -EPERM;
	if (!LEGAL_VPORT(esw, vport))
		return -EINVAL;

	mutex_lock(&esw->state_lock);
	evport = &esw->vports[vport];
	pschk = evport->info.spoofchk;
	evport->info.spoofchk = spoofchk;
	if (evport->enabled && esw->mode == SRIOV_LEGACY)
		err = esw_vport_ingress_config(esw, evport);
	if (err)
		evport->info.spoofchk = pschk;
	mutex_unlock(&esw->state_lock);

	return err;
}

int mlx5_eswitch_set_vport_trust(struct mlx5_eswitch *esw,
				 int vport, bool setting)
{
	struct mlx5_vport *evport;

	if (!ESW_ALLOWED(esw))
		return -EPERM;
	if (!LEGAL_VPORT(esw, vport))
		return -EINVAL;

	mutex_lock(&esw->state_lock);
	evport = &esw->vports[vport];
	evport->info.trusted = setting;
	if (evport->enabled)
		esw_vport_change_handle_locked(evport);
	mutex_unlock(&esw->state_lock);

	return 0;
}

static u32 calculate_vports_min_rate_divider(struct mlx5_eswitch *esw)
{
	u32 fw_max_bw_share = MLX5_CAP_QOS(esw->dev, max_tsar_bw_share);
	struct mlx5_vport *evport;
	u32 max_guarantee = 0;
	int i;

	for (i = 0; i < esw->total_vports; i++) {
		evport = &esw->vports[i];
		if (!evport->enabled || evport->info.min_rate < max_guarantee)
			continue;
		max_guarantee = evport->info.min_rate;
	}