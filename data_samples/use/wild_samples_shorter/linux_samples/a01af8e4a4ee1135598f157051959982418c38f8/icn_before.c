static int __init icn_init(void)
{
	char *p;
	char rev[10];

	memset(&dev, 0, sizeof(icn_dev));
	dev.memaddr = (membase & 0x0ffc000);
	dev.channel = -1;
	spin_lock_init(&dev.devlock);

	if ((p = strchr(revision, ':'))) {
		strcpy(rev, p + 1);
		p = strchr(rev, '$');
		*p = 0;
	} else
		strcpy(rev, " ??? ");
	printk(KERN_NOTICE "ICN-ISDN-driver Rev%smem=0x%08lx\n", rev,
	       dev.memaddr);