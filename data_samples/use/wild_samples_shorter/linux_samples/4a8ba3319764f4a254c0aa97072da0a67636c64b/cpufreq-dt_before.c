	/* OPPs might be populated at runtime, don't check for error here */
	of_init_opp_table(cpu_dev);

	priv = kzalloc(sizeof(*priv), GFP_KERNEL);
	if (!priv) {
		ret = -ENOMEM;
		goto out_free_opp;