static int linear_read_system(struct x86_emulate_ctxt *ctxt, ulong linear,
			      void *data, unsigned size)
{
	return ctxt->ops->read_std(ctxt, linear, data, size, &ctxt->exception, true);
}

static int linear_write_system(struct x86_emulate_ctxt *ctxt,
			       ulong linear, void *data,
			       unsigned int size)
{
	return ctxt->ops->write_std(ctxt, linear, data, size, &ctxt->exception, true);
}

static int segmented_read_std(struct x86_emulate_ctxt *ctxt,
			      struct segmented_address addr,
	rc = linearize(ctxt, addr, size, false, &linear);
	if (rc != X86EMUL_CONTINUE)
		return rc;
	return ctxt->ops->read_std(ctxt, linear, data, size, &ctxt->exception, false);
}

static int segmented_write_std(struct x86_emulate_ctxt *ctxt,
			       struct segmented_address addr,
	rc = linearize(ctxt, addr, size, true, &linear);
	if (rc != X86EMUL_CONTINUE)
		return rc;
	return ctxt->ops->write_std(ctxt, linear, data, size, &ctxt->exception, false);
}

/*
 * Prefetch the remaining bytes of the instruction without crossing page
#ifdef CONFIG_X86_64
	base |= ((u64)base3) << 32;
#endif
	r = ops->read_std(ctxt, base + 102, &io_bitmap_ptr, 2, NULL, true);
	if (r != X86EMUL_CONTINUE)
		return false;
	if (io_bitmap_ptr + port/8 > desc_limit_scaled(&tr_seg))
		return false;
	r = ops->read_std(ctxt, base + io_bitmap_ptr + port/8, &perm, 2, NULL, true);
	if (r != X86EMUL_CONTINUE)
		return false;
	if ((perm >> bit_idx) & mask)
		return false;