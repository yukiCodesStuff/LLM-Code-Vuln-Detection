{
	char *p;
	char rev[20];

	memset(&dev, 0, sizeof(icn_dev));
	dev.memaddr = (membase & 0x0ffc000);
	dev.channel = -1;
	dev.mcard = NULL;
	dev.firstload = 1;
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
	return (icn_addcard(portbase, icn_id, icn_id2));
}

static void __exit icn_exit(void)
{
	isdn_ctrl cmd;
	icn_card *card = cards;
	icn_card *last, *tmpcard;
	int i;
	unsigned long flags;

	icn_stopallcards();
	while (card) {
		cmd.command = ISDN_STAT_UNLOAD;
		cmd.driver = card->myid;
		card->interface.statcallb(&cmd);
		spin_lock_irqsave(&card->lock, flags);
		if (card->rvalid) {
			OUTB_P(0, ICN_RUN);	/* Reset Controller     */
			OUTB_P(0, ICN_MAPRAM);	/* Disable RAM          */
			if (card->secondhalf || (!card->doubleS0)) {
				release_region(card->port, ICN_PORTLEN);
				card->rvalid = 0;
			}
			for (i = 0; i < ICN_BCH; i++)
				icn_free_queue(card, i);
		}
		tmpcard = card->next;
		spin_unlock_irqrestore(&card->lock, flags);
		card = tmpcard;
	}
	card = cards;
	cards = NULL;
	while (card) {
		last = card;
		card = card->next;
		kfree(last);
	}
	if (dev.mvalid) {
		iounmap(dev.shmem);
		release_mem_region(dev.memaddr, 0x4000);
	}
	printk(KERN_NOTICE "ICN-ISDN-driver unloaded\n");
}

module_init(icn_init);
module_exit(icn_exit);
{
	char *p;
	char rev[20];

	memset(&dev, 0, sizeof(icn_dev));
	dev.memaddr = (membase & 0x0ffc000);
	dev.channel = -1;
	dev.mcard = NULL;
	dev.firstload = 1;
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
	return (icn_addcard(portbase, icn_id, icn_id2));
}