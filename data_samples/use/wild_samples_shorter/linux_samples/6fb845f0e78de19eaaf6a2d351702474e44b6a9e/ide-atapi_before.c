
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
EXPORT_SYMBOL_GPL(ide_queue_sense_rq);
