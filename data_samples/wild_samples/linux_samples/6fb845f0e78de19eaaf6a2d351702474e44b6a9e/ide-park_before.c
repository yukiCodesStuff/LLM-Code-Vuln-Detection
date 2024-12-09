	if (drive->dev_flags & IDE_DFLAG_PARKED) {
		int reset_timer = time_before(timeout, drive->sleep);
		int start_queue = 0;

		drive->sleep = timeout;
		wake_up_all(&ide_park_wq);
		if (reset_timer && del_timer(&hwif->timer))
			start_queue = 1;
		spin_unlock_irq(&hwif->lock);

		if (start_queue)
			blk_mq_run_hw_queues(q, true);
		return;
	}
	spin_unlock_irq(&hwif->lock);

	rq = blk_get_request(q, REQ_OP_DRV_IN, 0);
	scsi_req(rq)->cmd[0] = REQ_PARK_HEADS;
	scsi_req(rq)->cmd_len = 1;
	ide_req(rq)->type = ATA_PRIV_MISC;
	ide_req(rq)->special = &timeout;
	blk_execute_rq(q, NULL, rq, 1);
	rc = scsi_req(rq)->result ? -EIO : 0;
	blk_put_request(rq);
	if (rc)
		goto out;

	/*
	 * Make sure that *some* command is sent to the drive after the
	 * timeout has expired, so power management will be reenabled.
	 */
	rq = blk_get_request(q, REQ_OP_DRV_IN, BLK_MQ_REQ_NOWAIT);
	if (IS_ERR(rq))
		goto out;

	scsi_req(rq)->cmd[0] = REQ_UNPARK_HEADS;
	scsi_req(rq)->cmd_len = 1;
	ide_req(rq)->type = ATA_PRIV_MISC;
	ide_insert_request_head(drive, rq);

out:
	return;
}

ide_startstop_t ide_do_park_unpark(ide_drive_t *drive, struct request *rq)
{
	struct ide_cmd cmd;
	struct ide_taskfile *tf = &cmd.tf;

	memset(&cmd, 0, sizeof(cmd));
	if (scsi_req(rq)->cmd[0] == REQ_PARK_HEADS) {
		drive->sleep = *(unsigned long *)ide_req(rq)->special;
		drive->dev_flags |= IDE_DFLAG_SLEEPING;
		tf->command = ATA_CMD_IDLEIMMEDIATE;
		tf->feature = 0x44;
		tf->lbal = 0x4c;
		tf->lbam = 0x4e;
		tf->lbah = 0x55;
		cmd.valid.out.tf = IDE_VALID_OUT_TF | IDE_VALID_DEVICE;
		cmd.valid.in.tf  = IDE_VALID_IN_TF  | IDE_VALID_DEVICE;
	} else		/* cmd == REQ_UNPARK_HEADS */
		tf->command = ATA_CMD_CHK_POWER;

	cmd.tf_flags |= IDE_TFLAG_CUSTOM_HANDLER;
	cmd.protocol = ATA_PROT_NODATA;

	cmd.rq = rq;

	return do_rw_taskfile(drive, &cmd);
}