	    drive->retry_pio <= 3) {
		drive->dev_flags &= ~IDE_DFLAG_DMA_PIO_RETRY;
		ide_dma_on(drive);
	}

	if (!blk_update_request(rq, error, nr_bytes)) {
		if (rq == drive->sense_rq)
			drive->sense_rq = NULL;

		__blk_mq_end_request(rq, error);
		return 0;
	}
	if (rq) {
		blk_mq_requeue_request(rq, false);
		blk_mq_delay_kick_requeue_list(q, 3);
	} else
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

	blk_mq_start_request(rq);

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
		spin_unlock_irq(&hwif->lock);
		ide_unlock_host(host);
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
	} else {
plug_device:
		spin_unlock_irq(&hwif->lock);
		ide_unlock_host(host);
		ide_requeue_and_plug(drive, rq);
		return BLK_STS_OK;
	}
	} else {
plug_device:
		spin_unlock_irq(&hwif->lock);
		ide_unlock_host(host);
		ide_requeue_and_plug(drive, rq);
		return BLK_STS_OK;
	}

out:
	spin_unlock_irq(&hwif->lock);
	if (rq == NULL)
		ide_unlock_host(host);
	return BLK_STS_OK;
}

static int drive_is_ready(ide_drive_t *drive)
{
	ide_hwif_t *hwif = drive->hwif;
	u8 stat = 0;

	if (drive->waiting_for_dma)
		return hwif->dma_ops->dma_test_irq(drive);

	if (hwif->io_ports.ctl_addr &&
	    (hwif->host_flags & IDE_HFLAG_BROKEN_ALTSTATUS) == 0)
		stat = hwif->tp_ops->read_altstatus(hwif);
	else
		/* Note: this may clear a pending IRQ!! */
		stat = hwif->tp_ops->read_status(hwif);

	if (stat & ATA_BUSY)
		/* drive busy: definitely not interrupting */
		return 0;

	/* drive ready: *might* be interrupting */
	return 1;
}

/**
 *	ide_timer_expiry	-	handle lack of an IDE interrupt
 *	@data: timer callback magic (hwif)
 *
 *	An IDE command has timed out before the expected drive return
 *	occurred. At this point we attempt to clean up the current
 *	mess. If the current handler includes an expiry handler then
 *	we invoke the expiry handler, and providing it is happy the
 *	work is done. If that fails we apply generic recovery rules
 *	invoking the handler and checking the drive DMA status. We
 *	have an excessively incestuous relationship with the DMA
 *	logic that wants cleaning up.
 */
 
void ide_timer_expiry (struct timer_list *t)
{
	ide_hwif_t	*hwif = from_timer(hwif, t, timer);
	ide_drive_t	*uninitialized_var(drive);
	ide_handler_t	*handler;
	unsigned long	flags;
	int		wait = -1;
	int		plug_device = 0;
	struct request	*uninitialized_var(rq_in_flight);

	spin_lock_irqsave(&hwif->lock, flags);

	handler = hwif->handler;

	if (handler == NULL || hwif->req_gen != hwif->req_gen_timer) {
		/*
		 * Either a marginal timeout occurred
		 * (got the interrupt just as timer expired),
		 * or we were "sleeping" to give other devices a chance.
		 * Either way, we don't really want to complain about anything.
		 */
	} else {
		ide_expiry_t *expiry = hwif->expiry;
		ide_startstop_t startstop = ide_stopped;

		drive = hwif->cur_dev;

		if (expiry) {
			wait = expiry(drive);
			if (wait > 0) { /* continue */
				/* reset timer */
				hwif->timer.expires = jiffies + wait;
				hwif->req_gen_timer = hwif->req_gen;
				add_timer(&hwif->timer);
				spin_unlock_irqrestore(&hwif->lock, flags);
				return;
			}
		}
		hwif->handler = NULL;
		hwif->expiry = NULL;
		/*
		 * We need to simulate a real interrupt when invoking
		 * the handler() function, which means we need to
		 * globally mask the specific IRQ:
		 */
		spin_unlock(&hwif->lock);
		/* disable_irq_nosync ?? */
		disable_irq(hwif->irq);

		if (hwif->polling) {
			startstop = handler(drive);
		} else if (drive_is_ready(drive)) {
			if (drive->waiting_for_dma)
				hwif->dma_ops->dma_lost_irq(drive);
			if (hwif->port_ops && hwif->port_ops->clear_irq)
				hwif->port_ops->clear_irq(drive);

			printk(KERN_WARNING "%s: lost interrupt\n",
				drive->name);
			startstop = handler(drive);
		} else {
			if (drive->waiting_for_dma)
				startstop = ide_dma_timeout_retry(drive, wait);
			else
				startstop = ide_error(drive, "irq timeout",
					hwif->tp_ops->read_status(hwif));
		}
		/* Disable interrupts again, `handler' might have enabled it */
		spin_lock_irq(&hwif->lock);
		enable_irq(hwif->irq);
		if (startstop == ide_stopped && hwif->polling == 0) {
			rq_in_flight = hwif->rq;
			hwif->rq = NULL;
			ide_unlock_port(hwif);
			plug_device = 1;
		}
	}
	spin_unlock_irqrestore(&hwif->lock, flags);

	if (plug_device) {
		ide_unlock_host(hwif->host);
		ide_requeue_and_plug(drive, rq_in_flight);
	}
}

/**
 *	unexpected_intr		-	handle an unexpected IDE interrupt
 *	@irq: interrupt line
 *	@hwif: port being processed
 *
 *	There's nothing really useful we can do with an unexpected interrupt,
 *	other than reading the status register (to clear it), and logging it.
 *	There should be no way that an irq can happen before we're ready for it,
 *	so we needn't worry much about losing an "important" interrupt here.
 *
 *	On laptops (and "green" PCs), an unexpected interrupt occurs whenever
 *	the drive enters "idle", "standby", or "sleep" mode, so if the status
 *	looks "good", we just ignore the interrupt completely.
 *
 *	This routine assumes __cli() is in effect when called.
 *
 *	If an unexpected interrupt happens on irq15 while we are handling irq14
 *	and if the two interfaces are "serialized" (CMD640), then it looks like
 *	we could screw up by interfering with a new request being set up for 
 *	irq15.
 *
 *	In reality, this is a non-issue.  The new command is not sent unless 
 *	the drive is ready to accept one, in which case we know the drive is
 *	not trying to interrupt us.  And ide_set_handler() is always invoked
 *	before completing the issuance of any new drive command, so we will not
 *	be accidentally invoked as a result of any valid command completion
 *	interrupt.
 */

static void unexpected_intr(int irq, ide_hwif_t *hwif)
{
	u8 stat = hwif->tp_ops->read_status(hwif);

	if (!OK_STAT(stat, ATA_DRDY, BAD_STAT)) {
		/* Try to not flood the console with msgs */
		static unsigned long last_msgtime, count;
		++count;

		if (time_after(jiffies, last_msgtime + HZ)) {
			last_msgtime = jiffies;
			printk(KERN_ERR "%s: unexpected interrupt, "
				"status=0x%02x, count=%ld\n",
				hwif->name, stat, count);
		}
	}
}

/**
 *	ide_intr	-	default IDE interrupt handler
 *	@irq: interrupt number
 *	@dev_id: hwif
 *	@regs: unused weirdness from the kernel irq layer
 *
 *	This is the default IRQ handler for the IDE layer. You should
 *	not need to override it. If you do be aware it is subtle in
 *	places
 *
 *	hwif is the interface in the group currently performing
 *	a command. hwif->cur_dev is the drive and hwif->handler is
 *	the IRQ handler to call. As we issue a command the handlers
 *	step through multiple states, reassigning the handler to the
 *	next step in the process. Unlike a smart SCSI controller IDE
 *	expects the main processor to sequence the various transfer
 *	stages. We also manage a poll timer to catch up with most
 *	timeout situations. There are still a few where the handlers
 *	don't ever decide to give up.
 *
 *	The handler eventually returns ide_stopped to indicate the
 *	request completed. At this point we issue the next request
 *	on the port and the process begins again.
 */

irqreturn_t ide_intr (int irq, void *dev_id)
{
	ide_hwif_t *hwif = (ide_hwif_t *)dev_id;
	struct ide_host *host = hwif->host;
	ide_drive_t *uninitialized_var(drive);
	ide_handler_t *handler;
	unsigned long flags;
	ide_startstop_t startstop;
	irqreturn_t irq_ret = IRQ_NONE;
	int plug_device = 0;
	struct request *uninitialized_var(rq_in_flight);

	if (host->host_flags & IDE_HFLAG_SERIALIZE) {
		if (hwif != host->cur_port)
			goto out_early;
	}

	spin_lock_irqsave(&hwif->lock, flags);

	if (hwif->port_ops && hwif->port_ops->test_irq &&
	    hwif->port_ops->test_irq(hwif) == 0)
		goto out;

	handler = hwif->handler;

	if (handler == NULL || hwif->polling) {
		/*
		 * Not expecting an interrupt from this drive.
		 * That means this could be:
		 *	(1) an interrupt from another PCI device
		 *	sharing the same PCI INT# as us.
		 * or	(2) a drive just entered sleep or standby mode,
		 *	and is interrupting to let us know.
		 * or	(3) a spurious interrupt of unknown origin.
		 *
		 * For PCI, we cannot tell the difference,
		 * so in that case we just ignore it and hope it goes away.
		 */
		if ((host->irq_flags & IRQF_SHARED) == 0) {
			/*
			 * Probably not a shared PCI interrupt,
			 * so we can safely try to do something about it:
			 */
			unexpected_intr(irq, hwif);
		} else {
			/*
			 * Whack the status register, just in case
			 * we have a leftover pending IRQ.
			 */
			(void)hwif->tp_ops->read_status(hwif);
		}
		goto out;
	}

	drive = hwif->cur_dev;

	if (!drive_is_ready(drive))
		/*
		 * This happens regularly when we share a PCI IRQ with
		 * another device.  Unfortunately, it can also happen
		 * with some buggy drives that trigger the IRQ before
		 * their status register is up to date.  Hopefully we have
		 * enough advance overhead that the latter isn't a problem.
		 */
		goto out;

	hwif->handler = NULL;
	hwif->expiry = NULL;
	hwif->req_gen++;
	del_timer(&hwif->timer);
	spin_unlock(&hwif->lock);

	if (hwif->port_ops && hwif->port_ops->clear_irq)
		hwif->port_ops->clear_irq(drive);

	if (drive->dev_flags & IDE_DFLAG_UNMASK)
		local_irq_enable_in_hardirq();

	/* service this interrupt, may set handler for next interrupt */
	startstop = handler(drive);

	spin_lock_irq(&hwif->lock);
	/*
	 * Note that handler() may have set things up for another
	 * interrupt to occur soon, but it cannot happen until
	 * we exit from this routine, because it will be the
	 * same irq as is currently being serviced here, and Linux
	 * won't allow another of the same (on any CPU) until we return.
	 */
	if (startstop == ide_stopped && hwif->polling == 0) {
		BUG_ON(hwif->handler);
		rq_in_flight = hwif->rq;
		hwif->rq = NULL;
		ide_unlock_port(hwif);
		plug_device = 1;
	}
	irq_ret = IRQ_HANDLED;
out:
	spin_unlock_irqrestore(&hwif->lock, flags);
out_early:
	if (plug_device) {
		ide_unlock_host(hwif->host);
		ide_requeue_and_plug(drive, rq_in_flight);
	}

	return irq_ret;
}
EXPORT_SYMBOL_GPL(ide_intr);

void ide_pad_transfer(ide_drive_t *drive, int write, int len)
{
	ide_hwif_t *hwif = drive->hwif;
	u8 buf[4] = { 0 };

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
	ide_hwif_t *hwif = drive->hwif;
	unsigned long flags;

	spin_lock_irqsave(&hwif->lock, flags);
	list_add_tail(&rq->queuelist, &drive->rq_list);
	spin_unlock_irqrestore(&hwif->lock, flags);

	kblockd_schedule_work(&drive->rq_work);
}
EXPORT_SYMBOL_GPL(ide_insert_request_head);
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
	ide_hwif_t *hwif = drive->hwif;
	unsigned long flags;

	spin_lock_irqsave(&hwif->lock, flags);
	list_add_tail(&rq->queuelist, &drive->rq_list);
	spin_unlock_irqrestore(&hwif->lock, flags);

	kblockd_schedule_work(&drive->rq_work);
}