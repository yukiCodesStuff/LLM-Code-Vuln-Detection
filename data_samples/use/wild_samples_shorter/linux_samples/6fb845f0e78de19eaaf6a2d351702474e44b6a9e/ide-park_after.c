	scsi_req(rq)->cmd[0] = REQ_UNPARK_HEADS;
	scsi_req(rq)->cmd_len = 1;
	ide_req(rq)->type = ATA_PRIV_MISC;
	spin_lock_irq(&hwif->lock);
	ide_insert_request_head(drive, rq);
	spin_unlock_irq(&hwif->lock);

out:
	return;
}