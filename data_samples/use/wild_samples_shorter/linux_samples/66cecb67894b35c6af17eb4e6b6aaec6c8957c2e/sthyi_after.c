	if (r < 0)
		goto out;

	diag224_buf = (void *)__get_free_page(GFP_KERNEL | GFP_DMA);
	if (!diag224_buf || diag224(diag224_buf))
		goto out;

	ti_hdr = diag204_buf;
	sctns->par.infpval1 |= PAR_WGHT_VLD;

out:
	free_page((unsigned long)diag224_buf);
	vfree(diag204_buf);
}

static int sthyi(u64 vaddr)