	DPRINTK("ENTER\n");

	cancel_delayed_work_sync(&ap->sff_pio_task);
	ap->hsm_task_state = HSM_ST_IDLE;
	ap->sff_pio_task_link = NULL;

	if (ata_msg_ctl(ap))
		ata_port_dbg(ap, "%s: EXIT\n", __func__);