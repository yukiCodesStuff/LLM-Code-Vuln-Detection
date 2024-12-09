	if (unlikely(err)) {
		if (printk_ratelimit())
			printk(KERN_WARNING PFX "%s: failed to map sense "
					    "buffer\n", drive->name);
		blk_mq_free_request(sense_rq);
		drive->sense_rq = NULL;
		return;
	}

	sense_rq->rq_disk = rq->rq_disk;
	sense_rq->cmd_flags = REQ_OP_DRV_IN;
	ide_req(sense_rq)->type = ATA_PRIV_SENSE;
	sense_rq->rq_flags |= RQF_PREEMPT;

	req->cmd[0] = GPCMD_REQUEST_SENSE;
	req->cmd[4] = cmd_len;
	if (drive->media == ide_tape)
		req->cmd[13] = REQ_IDETAPE_PC1;

	drive->sense_rq_armed = true;
}
EXPORT_SYMBOL_GPL(ide_prep_sense);

int ide_queue_sense_rq(ide_drive_t *drive, void *special)
{
	struct request *sense_rq = drive->sense_rq;

	/* deferred failure from ide_prep_sense() */
	if (!drive->sense_rq_armed) {
		printk(KERN_WARNING PFX "%s: error queuing a sense request\n",
		       drive->name);
		return -ENOMEM;
	}

	ide_req(sense_rq)->special = special;
	drive->sense_rq_armed = false;

	drive->hwif->rq = NULL;

	ide_insert_request_head(drive, sense_rq);
	return 0;
}