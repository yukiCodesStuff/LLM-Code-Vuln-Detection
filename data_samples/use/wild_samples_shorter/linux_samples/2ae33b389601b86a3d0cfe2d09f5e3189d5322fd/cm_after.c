	INIT_LIST_HEAD(&dev->sriov.cm_list);
	dev->sriov.sl_id_map = RB_ROOT;
	idr_init(&dev->sriov.pv_id_table);
}

/* slave = -1 ==> all slaves */
/* TBD -- call paravirt clean for single slave.  Need for slave RESET event */