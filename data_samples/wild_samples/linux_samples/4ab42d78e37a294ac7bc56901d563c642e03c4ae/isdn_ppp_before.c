	if (slot < 0) {
		return -EBUSY;
	}
	is = file->private_data = ippp_table[slot];

	printk(KERN_DEBUG "ippp, open, slot: %d, minor: %d, state: %04x\n",
	       slot, min, is->state);

	/* compression stuff */
	is->link_compressor   = is->compressor = NULL;
	is->link_decompressor = is->decompressor = NULL;
	is->link_comp_stat    = is->comp_stat = NULL;
	is->link_decomp_stat  = is->decomp_stat = NULL;
	is->compflags = 0;

	is->reset = isdn_ppp_ccp_reset_alloc(is);
	if (!is->reset)
		return -ENOMEM;

	is->lp = NULL;
	is->mp_seqno = 0;       /* MP sequence number */
	is->pppcfg = 0;         /* ppp configuration */
	is->mpppcfg = 0;        /* mppp configuration */
	is->last_link_seqno = -1;	/* MP: maybe set to Bundle-MIN, when joining a bundle ?? */
	is->unit = -1;          /* set, when we have our interface */
	is->mru = 1524;         /* MRU, default 1524 */
	is->maxcid = 16;        /* VJ: maxcid */
	is->tk = current;
	init_waitqueue_head(&is->wq);
	is->first = is->rq + NUM_RCV_BUFFS - 1;	/* receive queue */
	is->last = is->rq;
	is->minor = min;
#ifdef CONFIG_ISDN_PPP_VJ
	/*
	 * VJ header compression init
	 */
	is->slcomp = slhc_init(16, 16);	/* not necessary for 2. link in bundle */
	if (!is->slcomp) {
		isdn_ppp_ccp_reset_free(is);
		return -ENOMEM;
	}
		if (is->maxcid != val) {
#ifdef CONFIG_ISDN_PPP_VJ
			struct slcompress *sltmp;
#endif
			if (is->debug & 0x1)
				printk(KERN_DEBUG "ippp, ioctl: changed MAXCID to %ld\n", val);
			is->maxcid = val;
#ifdef CONFIG_ISDN_PPP_VJ
			sltmp = slhc_init(16, val);
			if (!sltmp) {
				printk(KERN_ERR "ippp, can't realloc slhc struct\n");
				return -ENOMEM;
			}