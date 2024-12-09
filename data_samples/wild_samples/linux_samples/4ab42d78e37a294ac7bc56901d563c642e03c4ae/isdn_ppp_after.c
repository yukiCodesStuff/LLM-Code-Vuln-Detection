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
	if (IS_ERR(is->slcomp)) {
		isdn_ppp_ccp_reset_free(is);
		return PTR_ERR(is->slcomp);
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
			if (IS_ERR(sltmp))
				return PTR_ERR(sltmp);
			if (is->slcomp)
				slhc_free(is->slcomp);
			is->slcomp = sltmp;
#endif
		}
		break;
	case PPPIOCGDEBUG:
		if ((r = set_arg(argp, &is->debug, sizeof(is->debug))))
			return r;
		break;
	case PPPIOCSDEBUG:
		if ((r = get_arg(argp, &val, sizeof(val))))
			return r;
		is->debug = val;
		break;
	case PPPIOCGCOMPRESSORS:
	{
		unsigned long protos[8] = {0,};
		struct isdn_ppp_compressor *ipc = ipc_head;
		while (ipc) {
			j = ipc->num / (sizeof(long) * 8);
			i = ipc->num % (sizeof(long) * 8);
			if (j < 8)
				protos[j] |= (1UL << i);
			ipc = ipc->next;
		}