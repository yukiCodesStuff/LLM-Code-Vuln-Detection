	ide_drive_t *drive = container_of(work, ide_drive_t, rq_work);
	ide_hwif_t *hwif = drive->hwif;
	struct request *rq;
	blk_status_t ret;
	LIST_HEAD(list);

	blk_mq_quiesce_queue(drive->queue);

	ret = BLK_STS_OK;
	spin_lock_irq(&hwif->lock);
	while (!list_empty(&drive->rq_list)) {
		rq = list_first_entry(&drive->rq_list, struct request, queuelist);
		list_del_init(&rq->queuelist);

		spin_unlock_irq(&hwif->lock);
		ret = ide_issue_rq(drive, rq, true);
		spin_lock_irq(&hwif->lock);
	}
	spin_unlock_irq(&hwif->lock);

	blk_mq_unquiesce_queue(drive->queue);

	if (ret != BLK_STS_OK)
		kblockd_schedule_work(&drive->rq_work);
}

static const u8 ide_hwif_to_major[] =
	{ IDE0_MAJOR, IDE1_MAJOR, IDE2_MAJOR, IDE3_MAJOR, IDE4_MAJOR,