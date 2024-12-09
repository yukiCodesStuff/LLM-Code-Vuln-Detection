	 * VJ header compression init
	 */
	is->slcomp = slhc_init(16, 16);	/* not necessary for 2. link in bundle */
	if (!is->slcomp) {
		isdn_ppp_ccp_reset_free(is);
		return -ENOMEM;
	}
#endif
#ifdef CONFIG_IPPP_FILTER
	is->pass_filter = NULL;
			is->maxcid = val;
#ifdef CONFIG_ISDN_PPP_VJ
			sltmp = slhc_init(16, val);
			if (!sltmp) {
				printk(KERN_ERR "ippp, can't realloc slhc struct\n");
				return -ENOMEM;
			}
			if (is->slcomp)
				slhc_free(is->slcomp);
			is->slcomp = sltmp;
#endif