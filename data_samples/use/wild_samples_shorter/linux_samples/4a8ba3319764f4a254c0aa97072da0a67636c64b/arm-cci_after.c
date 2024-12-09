	if (!np)
		return -ENODEV;

	if (!of_device_is_available(np))
		return -ENODEV;

	cci_config = of_match_node(arm_cci_matches, np)->data;
	if (!cci_config)
		return -ENODEV;
