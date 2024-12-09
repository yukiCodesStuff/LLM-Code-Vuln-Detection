	}

	if (!blk_update_request(rq, error, nr_bytes)) {
		if (rq == drive->sense_rq) {
			drive->sense_rq = NULL;
			drive->sense_rq_active = false;
		}

		__blk_mq_end_request(rq, error);
		return 0;
	}
		blk_mq_delay_run_hw_queue(q->queue_hw_ctx[0], 3);
}

blk_status_t ide_issue_rq(ide_drive_t *drive, struct request *rq,
			  bool local_requeue)
{
	ide_hwif_t *hwif = drive->hwif;
	struct ide_host *host = hwif->host;
	ide_startstop_t	startstop;

	if (!blk_rq_is_passthrough(rq) && !(rq->rq_flags & RQF_DONTPREP)) {
		rq->rq_flags |= RQF_DONTPREP;
	if (ide_lock_host(host, hwif))
		return BLK_STS_DEV_RESOURCE;

	spin_lock_irq(&hwif->lock);

	if (!ide_lock_port(hwif)) {
		ide_hwif_t *prev_port;
		hwif->cur_dev = drive;
		drive->dev_flags &= ~(IDE_DFLAG_SLEEPING | IDE_DFLAG_PARKED);

		/*
		 * Sanity: don't accept a request that isn't a PM request
		 * if we are currently power managed. This is very important as
		 * blk_stop_queue() doesn't prevent the blk_fetch_request()
		}
	} else {
plug_device:
		if (local_requeue)
			list_add(&rq->queuelist, &drive->rq_list);
		spin_unlock_irq(&hwif->lock);
		ide_unlock_host(host);
		if (!local_requeue)
			ide_requeue_and_plug(drive, rq);
		return BLK_STS_OK;
	}

out:
	return BLK_STS_OK;
}

/*
 * Issue a new request to a device.
 */
blk_status_t ide_queue_rq(struct blk_mq_hw_ctx *hctx,
			  const struct blk_mq_queue_data *bd)
{
	ide_drive_t *drive = hctx->queue->queuedata;
	ide_hwif_t *hwif = drive->hwif;

	spin_lock_irq(&hwif->lock);
	if (drive->sense_rq_active) {
		spin_unlock_irq(&hwif->lock);
		return BLK_STS_DEV_RESOURCE;
	}
	spin_unlock_irq(&hwif->lock);

	blk_mq_start_request(bd->rq);
	return ide_issue_rq(drive, bd->rq, false);
}

static int drive_is_ready(ide_drive_t *drive)
{
	ide_hwif_t *hwif = drive->hwif;
	u8 stat = 0;

void ide_insert_request_head(ide_drive_t *drive, struct request *rq)
{
	drive->sense_rq_active = true;
	list_add_tail(&rq->queuelist, &drive->rq_list);
	kblockd_schedule_work(&drive->rq_work);
}
EXPORT_SYMBOL_GPL(ide_insert_request_head);