static int __init nfhd_init(void)
{
	u32 blocks, bsize;
	int ret;
	int i;

	nfhd_id = nf_get_id("XHDI");
	if (!nfhd_id)
		return -ENODEV;

	ret = register_blkdev(major_num, "nfhd");
	if (ret < 0) {
		pr_warn("nfhd: unable to get major number\n");
		return ret;
	}

	if (!major_num)
		major_num = ret;

	for (i = NFHD_DEV_OFFSET; i < 24; i++) {
		if (nfhd_get_capacity(i, 0, &blocks, &bsize))
			continue;
		nfhd_init_one(i, blocks, bsize);