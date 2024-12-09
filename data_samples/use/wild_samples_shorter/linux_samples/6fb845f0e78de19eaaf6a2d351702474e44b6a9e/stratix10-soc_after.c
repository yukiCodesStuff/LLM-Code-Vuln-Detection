		return -ENODEV;

	np = of_find_matching_node(fw_np, s10_of_match);
	if (!np)
		return -ENODEV;

	of_node_put(np);
	ret = of_platform_populate(fw_np, s10_of_match, NULL, NULL);
	if (ret)
		return ret;

	return platform_driver_register(&s10_driver);