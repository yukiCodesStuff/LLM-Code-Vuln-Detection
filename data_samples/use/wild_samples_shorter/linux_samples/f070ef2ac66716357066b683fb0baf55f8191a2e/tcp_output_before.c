	if (nsize < 0)
		nsize = 0;

	if (skb_unclone(skb, gfp))
		return -ENOMEM;

	/* Get a new skb... force flag on. */