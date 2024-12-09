static int __init nfhd_init(void)
{
	u32 blocks, bsize;
	int i;

	nfhd_id = nf_get_id("XHDI");
	if (!nfhd_id)
		return -ENODEV;

	major_num = register_blkdev(major_num, "nfhd");
	if (major_num <= 0) {
		pr_warn("nfhd: unable to get major number\n");
		return major_num;
	}

	for (i = NFHD_DEV_OFFSET; i < 24; i++) {
		if (nfhd_get_capacity(i, 0, &blocks, &bsize))
			continue;
		nfhd_init_one(i, blocks, bsize);