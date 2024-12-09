	if (dmasync)
		dma_set_attr(DMA_ATTR_WRITE_BARRIER, &attrs);

	/*
	 * If the combination of the addr and size requested for this memory
	 * region causes an integer overflow, return error.
	 */
	if ((PAGE_ALIGN(addr + size) <= size) ||
	    (PAGE_ALIGN(addr + size) <= addr))
		return ERR_PTR(-EINVAL);

	if (!can_do_mlock())
		return ERR_PTR(-EPERM);

	umem = kzalloc(sizeof *umem, GFP_KERNEL);