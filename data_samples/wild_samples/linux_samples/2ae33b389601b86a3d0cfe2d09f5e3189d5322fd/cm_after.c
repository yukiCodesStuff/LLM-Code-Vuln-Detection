{
	spin_lock_init(&dev->sriov.id_map_lock);
	INIT_LIST_HEAD(&dev->sriov.cm_list);
	dev->sriov.sl_id_map = RB_ROOT;
	idr_init(&dev->sriov.pv_id_table);
}

/* slave = -1 ==> all slaves */
/* TBD -- call paravirt clean for single slave.  Need for slave RESET event */
void mlx4_ib_cm_paravirt_clean(struct mlx4_ib_dev *dev, int slave)
{
	struct mlx4_ib_sriov *sriov = &dev->sriov;
	struct rb_root *sl_id_map = &sriov->sl_id_map;
	struct list_head lh;
	struct rb_node *nd;
	int need_flush = 1;
	struct id_map_entry *map, *tmp_map;
	/* cancel all delayed work queue entries */
	INIT_LIST_HEAD(&lh);
	spin_lock(&sriov->id_map_lock);
	list_for_each_entry_safe(map, tmp_map, &dev->sriov.cm_list, list) {
		if (slave < 0 || slave == map->slave_id) {
			if (map->scheduled_delete)
				need_flush &= !!cancel_delayed_work(&map->timeout);
		}
	}