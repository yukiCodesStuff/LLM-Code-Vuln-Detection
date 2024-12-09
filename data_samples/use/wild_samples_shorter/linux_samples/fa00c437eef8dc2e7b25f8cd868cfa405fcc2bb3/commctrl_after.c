	struct fib *fibptr;
	struct hw_fib * hw_fib = (struct hw_fib *)0;
	dma_addr_t hw_fib_pa = (dma_addr_t)0LL;
	unsigned int size, osize;
	int retval;

	if (dev->in_reset) {
		return -EBUSY;
	 *	will not overrun the buffer when we copy the memory. Return
	 *	an error if we would.
	 */
	osize = size = le16_to_cpu(kfib->header.Size) +
		sizeof(struct aac_fibhdr);
	if (size < le16_to_cpu(kfib->header.SenderSize))
		size = le16_to_cpu(kfib->header.SenderSize);
	if (size > dev->max_fib_size) {
		dma_addr_t daddr;
		goto cleanup;
	}

	/* Sanity check the second copy */
	if ((osize != le16_to_cpu(kfib->header.Size) +
		sizeof(struct aac_fibhdr))
		|| (size < le16_to_cpu(kfib->header.SenderSize))) {
		retval = -EINVAL;
		goto cleanup;
	}

	if (kfib->header.Command == cpu_to_le16(TakeABreakPt)) {
		aac_adapter_interrupt(dev);
		/*
		 * Since we didn't really send a fib, zero out the state to allow