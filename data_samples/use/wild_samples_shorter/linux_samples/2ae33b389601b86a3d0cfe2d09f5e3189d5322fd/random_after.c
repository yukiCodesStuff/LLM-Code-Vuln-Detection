		      int reserved)
{
	unsigned long flags;
	int wakeup_write = 0;

	/* Hold lock while accounting */
	spin_lock_irqsave(&r->lock, flags);

		else
			r->entropy_count = reserved;

		if (r->entropy_count < random_write_wakeup_thresh)
			wakeup_write = 1;
	}

	DEBUG_ENT("debiting %zu entropy credits from %s%s\n",
		  nbytes * 8, r->name, r->limit ? "" : " (unlimited)");

	spin_unlock_irqrestore(&r->lock, flags);

	if (wakeup_write) {
		wake_up_interruptible(&random_write_wait);
		kill_fasync(&fasync, SIGIO, POLL_OUT);
	}

	return nbytes;
}

static void extract_buf(struct entropy_store *r, __u8 *out)