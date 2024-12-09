	ide_drive_t *drive = container_of(work, ide_drive_t, rq_work);
	ide_hwif_t *hwif = drive->hwif;
	struct request *rq;
	LIST_HEAD(list);

	spin_lock_irq(&hwif->lock);
	if (!list_empty(&drive->rq_list))
		list_splice_init(&drive->rq_list, &list);
	spin_unlock_irq(&hwif->lock);

	while (!list_empty(&list)) {
		rq = list_first_entry(&list, struct request, queuelist);
		list_del_init(&rq->queuelist);
		blk_execute_rq_nowait(drive->queue, rq->rq_disk, rq, true, NULL);
	}
}

static const u8 ide_hwif_to_major[] =
	{ IDE0_MAJOR, IDE1_MAJOR, IDE2_MAJOR, IDE3_MAJOR, IDE4_MAJOR,