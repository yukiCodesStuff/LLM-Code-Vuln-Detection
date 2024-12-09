{
	/* nudge anyone waiting on suspend queue */
	if (unlikely(wq_has_sleeper(&md->wait)))
		wake_up(&md->wait);

	/*
	 * dm_put() must be at the end of this function. See the comment above
	 */
	dm_put(md);
}