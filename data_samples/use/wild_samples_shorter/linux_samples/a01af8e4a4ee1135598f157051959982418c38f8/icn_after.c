static int __init icn_init(void)
{
	char *p;
	char rev[20];

	memset(&dev, 0, sizeof(icn_dev));
	dev.memaddr = (membase & 0x0ffc000);
	dev.channel = -1;
	spin_lock_init(&dev.devlock);

	if ((p = strchr(revision, ':'))) {
		strncpy(rev, p + 1, 20);
		p = strchr(rev, '$');
		if (p)
			*p = 0;
	} else
		strcpy(rev, " ??? ");
	printk(KERN_NOTICE "ICN-ISDN-driver Rev%smem=0x%08lx\n", rev,
	       dev.memaddr);