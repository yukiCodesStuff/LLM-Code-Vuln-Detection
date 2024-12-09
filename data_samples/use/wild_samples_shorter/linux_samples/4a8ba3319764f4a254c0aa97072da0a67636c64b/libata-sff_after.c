	DPRINTK("ENTER\n");

	cancel_delayed_work_sync(&ap->sff_pio_task);

	/*
	 * We wanna reset the HSM state to IDLE.  If we do so without
	 * grabbing the port lock, critical sections protected by it which
	 * expect the HSM state to stay stable may get surprised.  For
	 * example, we may set IDLE in between the time
	 * __ata_sff_port_intr() checks for HSM_ST_IDLE and before it calls
	 * ata_sff_hsm_move() causing ata_sff_hsm_move() to BUG().
	 */
	spin_lock_irq(ap->lock);
	ap->hsm_task_state = HSM_ST_IDLE;
	spin_unlock_irq(ap->lock);

	ap->sff_pio_task_link = NULL;

	if (ata_msg_ctl(ap))
		ata_port_dbg(ap, "%s: EXIT\n", __func__);