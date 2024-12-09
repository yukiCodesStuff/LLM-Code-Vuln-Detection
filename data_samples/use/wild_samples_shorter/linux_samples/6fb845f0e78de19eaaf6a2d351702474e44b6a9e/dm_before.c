				    true, duration, &io->stats_aux);

	/* nudge anyone waiting on suspend queue */
	if (unlikely(waitqueue_active(&md->wait)))
		wake_up(&md->wait);
}

/*
			return r;
	}

	bio_trim(clone, sector - clone->bi_iter.bi_sector, len);

	return 0;
}
