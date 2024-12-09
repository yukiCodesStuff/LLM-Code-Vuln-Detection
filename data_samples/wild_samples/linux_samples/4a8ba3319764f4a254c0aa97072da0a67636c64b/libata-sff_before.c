{
	DPRINTK("ENTER\n");

	cancel_delayed_work_sync(&ap->sff_pio_task);
	ap->hsm_task_state = HSM_ST_IDLE;
	ap->sff_pio_task_link = NULL;

	if (ata_msg_ctl(ap))
		ata_port_dbg(ap, "%s: EXIT\n", __func__);
}

static void ata_sff_pio_task(struct work_struct *work)
{
	struct ata_port *ap =
		container_of(work, struct ata_port, sff_pio_task.work);
	struct ata_link *link = ap->sff_pio_task_link;
	struct ata_queued_cmd *qc;
	u8 status;
	int poll_next;

	BUG_ON(ap->sff_pio_task_link == NULL);
	/* qc can be NULL if timeout occurred */
	qc = ata_qc_from_tag(ap, link->active_tag);
	if (!qc) {
		ap->sff_pio_task_link = NULL;
		return;
	}