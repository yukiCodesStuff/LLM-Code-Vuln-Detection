
int ide_queue_sense_rq(ide_drive_t *drive, void *special)
{
	ide_hwif_t *hwif = drive->hwif;
	struct request *sense_rq;
	unsigned long flags;

	spin_lock_irqsave(&hwif->lock, flags);

	/* deferred failure from ide_prep_sense() */
	if (!drive->sense_rq_armed) {
		printk(KERN_WARNING PFX "%s: error queuing a sense request\n",
		       drive->name);
		spin_unlock_irqrestore(&hwif->lock, flags);
		return -ENOMEM;
	}

	sense_rq = drive->sense_rq;
	ide_req(sense_rq)->special = special;
	drive->sense_rq_armed = false;

	drive->hwif->rq = NULL;

	ide_insert_request_head(drive, sense_rq);
	spin_unlock_irqrestore(&hwif->lock, flags);
	return 0;
}
EXPORT_SYMBOL_GPL(ide_queue_sense_rq);
