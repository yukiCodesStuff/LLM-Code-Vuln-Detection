	}

	if (!blk_update_request(rq, error, nr_bytes)) {
		if (rq == drive->sense_rq)
			drive->sense_rq = NULL;

		__blk_mq_end_request(rq, error);
		return 0;
	}
		blk_mq_delay_run_hw_queue(q->queue_hw_ctx[0], 3);
}

/*
 * Issue a new request to a device.
 */
blk_status_t ide_queue_rq(struct blk_mq_hw_ctx *hctx,
			  const struct blk_mq_queue_data *bd)
{
	ide_drive_t	*drive = hctx->queue->queuedata;
	ide_hwif_t	*hwif = drive->hwif;
	struct ide_host *host = hwif->host;
	struct request	*rq = bd->rq;
	ide_startstop_t	startstop;

	if (!blk_rq_is_passthrough(rq) && !(rq->rq_flags & RQF_DONTPREP)) {
		rq->rq_flags |= RQF_DONTPREP;
	if (ide_lock_host(host, hwif))
		return BLK_STS_DEV_RESOURCE;

	blk_mq_start_request(rq);

	spin_lock_irq(&hwif->lock);

	if (!ide_lock_port(hwif)) {
		ide_hwif_t *prev_port;
		hwif->cur_dev = drive;
		drive->dev_flags &= ~(IDE_DFLAG_SLEEPING | IDE_DFLAG_PARKED);

		/*
		 * we know that the queue isn't empty, but this can happen
		 * if ->prep_rq() decides to kill a request
		 */
		if (!rq) {
			rq = bd->rq;
			if (!rq) {
				ide_unlock_port(hwif);
				goto out;
			}
		}

		/*
		 * Sanity: don't accept a request that isn't a PM request
		 * if we are currently power managed. This is very important as
		 * blk_stop_queue() doesn't prevent the blk_fetch_request()
		}
	} else {
plug_device:
		spin_unlock_irq(&hwif->lock);
		ide_unlock_host(host);
		ide_requeue_and_plug(drive, rq);
		return BLK_STS_OK;
	}

out:
	return BLK_STS_OK;
}

static int drive_is_ready(ide_drive_t *drive)
{
	ide_hwif_t *hwif = drive->hwif;
	u8 stat = 0;

void ide_insert_request_head(ide_drive_t *drive, struct request *rq)
{
	ide_hwif_t *hwif = drive->hwif;
	unsigned long flags;

	spin_lock_irqsave(&hwif->lock, flags);
	list_add_tail(&rq->queuelist, &drive->rq_list);
	spin_unlock_irqrestore(&hwif->lock, flags);

	kblockd_schedule_work(&drive->rq_work);
}
EXPORT_SYMBOL_GPL(ide_insert_request_head);