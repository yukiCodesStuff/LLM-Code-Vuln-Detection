	blk_flush_complete_seq(rq, fq, REQ_FSEQ_DATA, error);
	spin_unlock_irqrestore(&fq->mq_flush_lock, flags);

	blk_mq_run_hw_queue(hctx, true);
}

/**
 * blk_insert_flush - insert a new PREFLUSH/FUA request