	    drive->retry_pio <= 3) {
		drive->dev_flags &= ~IDE_DFLAG_DMA_PIO_RETRY;
		ide_dma_on(drive);
	}

	if (!blk_update_request(rq, error, nr_bytes)) {
		if (rq == drive->sense_rq) {
			drive->sense_rq = NULL;
			drive->sense_rq_active = false;
		}

		__blk_mq_end_request(rq, error);
		return 0;
	}
	if (rq) {
		blk_mq_requeue_request(rq, false);
		blk_mq_delay_kick_requeue_list(q, 3);
	} else
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
		ide_req(rq)->special = NULL;
	}
	if (!blk_rq_is_passthrough(rq) && !(rq->rq_flags & RQF_DONTPREP)) {
		rq->rq_flags |= RQF_DONTPREP;
		ide_req(rq)->special = NULL;
	}

	/* HLD do_request() callback might sleep, make sure it's okay */
	might_sleep();

	if (ide_lock_host(host, hwif))
		return BLK_STS_DEV_RESOURCE;

	spin_lock_irq(&hwif->lock);

	if (!ide_lock_port(hwif)) {
		ide_hwif_t *prev_port;

		WARN_ON_ONCE(hwif->rq);
repeat:
		prev_port = hwif->host->cur_port;
		if (drive->dev_flags & IDE_DFLAG_SLEEPING &&
		    time_after(drive->sleep, jiffies)) {
			ide_unlock_port(hwif);
			goto plug_device;
		}

		if ((hwif->host->host_flags & IDE_HFLAG_SERIALIZE) &&
		    hwif != prev_port) {
			ide_drive_t *cur_dev =
				prev_port ? prev_port->cur_dev : NULL;

			/*
			 * set nIEN for previous port, drives in the
			 * quirk list may not like intr setups/cleanups
			 */
			if (cur_dev &&
			    (cur_dev->dev_flags & IDE_DFLAG_NIEN_QUIRK) == 0)
				prev_port->tp_ops->write_devctl(prev_port,
								ATA_NIEN |
								ATA_DEVCTL_OBS);

			hwif->host->cur_port = hwif;
		}
		hwif->cur_dev = drive;
		drive->dev_flags &= ~(IDE_DFLAG_SLEEPING | IDE_DFLAG_PARKED);

		/*
		 * Sanity: don't accept a request that isn't a PM request
		 * if we are currently power managed. This is very important as
		 * blk_stop_queue() doesn't prevent the blk_fetch_request()
		 * above to return us whatever is in the queue. Since we call
		 * ide_do_request() ourselves, we end up taking requests while
		 * the queue is blocked...
		 * 
		 * We let requests forced at head of queue with ide-preempt
		 * though. I hope that doesn't happen too much, hopefully not
		 * unless the subdriver triggers such a thing in its own PM
		 * state machine.
		 */
		if ((drive->dev_flags & IDE_DFLAG_BLOCKED) &&
		    ata_pm_request(rq) == 0 &&
		    (rq->rq_flags & RQF_PREEMPT) == 0) {
			/* there should be no pending command at this point */
			ide_unlock_port(hwif);
			goto plug_device;
		}

		scsi_req(rq)->resid_len = blk_rq_bytes(rq);
		hwif->rq = rq;

		spin_unlock_irq(&hwif->lock);
		startstop = start_request(drive, rq);
		spin_lock_irq(&hwif->lock);

		if (startstop == ide_stopped) {
			rq = hwif->rq;
			hwif->rq = NULL;
			if (rq)
				goto repeat;
			ide_unlock_port(hwif);
			goto out;
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
	spin_unlock_irq(&hwif->lock);
	if (rq == NULL)
		ide_unlock_host(host);
	return BLK_STS_OK;
}
		    hwif != prev_port) {
			ide_drive_t *cur_dev =
				prev_port ? prev_port->cur_dev : NULL;

			/*
			 * set nIEN for previous port, drives in the
			 * quirk list may not like intr setups/cleanups
			 */
			if (cur_dev &&
			    (cur_dev->dev_flags & IDE_DFLAG_NIEN_QUIRK) == 0)
				prev_port->tp_ops->write_devctl(prev_port,
								ATA_NIEN |
								ATA_DEVCTL_OBS);

			hwif->host->cur_port = hwif;
		}
		hwif->cur_dev = drive;
		drive->dev_flags &= ~(IDE_DFLAG_SLEEPING | IDE_DFLAG_PARKED);

		/*
		 * Sanity: don't accept a request that isn't a PM request
		 * if we are currently power managed. This is very important as
		 * blk_stop_queue() doesn't prevent the blk_fetch_request()
		 * above to return us whatever is in the queue. Since we call
		 * ide_do_request() ourselves, we end up taking requests while
		 * the queue is blocked...
		 * 
		 * We let requests forced at head of queue with ide-preempt
		 * though. I hope that doesn't happen too much, hopefully not
		 * unless the subdriver triggers such a thing in its own PM
		 * state machine.
		 */
		if ((drive->dev_flags & IDE_DFLAG_BLOCKED) &&
		    ata_pm_request(rq) == 0 &&
		    (rq->rq_flags & RQF_PREEMPT) == 0) {
			/* there should be no pending command at this point */
			ide_unlock_port(hwif);
			goto plug_device;
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
	spin_unlock_irq(&hwif->lock);
	if (rq == NULL)
		ide_unlock_host(host);
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
	while (len > 0) {
		if (write)
			hwif->tp_ops->output_data(drive, NULL, buf, min(4, len));
		else
			hwif->tp_ops->input_data(drive, NULL, buf, min(4, len));
		len -= 4;
	}
}
EXPORT_SYMBOL_GPL(ide_pad_transfer);

void ide_insert_request_head(ide_drive_t *drive, struct request *rq)
{
	drive->sense_rq_active = true;
	list_add_tail(&rq->queuelist, &drive->rq_list);
	kblockd_schedule_work(&drive->rq_work);
}