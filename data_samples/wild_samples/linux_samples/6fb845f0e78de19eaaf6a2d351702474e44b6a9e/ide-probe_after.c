{
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