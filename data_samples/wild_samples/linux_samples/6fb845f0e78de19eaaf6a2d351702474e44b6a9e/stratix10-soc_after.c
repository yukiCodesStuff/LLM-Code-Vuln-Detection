{
	struct device_node *fw_np;
	struct device_node *np;
	int ret;

	fw_np = of_find_node_by_name(NULL, "svc");
	if (!fw_np)
		return -ENODEV;

	np = of_find_matching_node(fw_np, s10_of_match);
	if (!np)
		return -ENODEV;

	of_node_put(np);
	ret = of_platform_populate(fw_np, s10_of_match, NULL, NULL);
	if (ret)
		return ret;

	return platform_driver_register(&s10_driver);
}

static void __exit s10_exit(void)
{
	return platform_driver_unregister(&s10_driver);
}

module_init(s10_init);
module_exit(s10_exit);

MODULE_AUTHOR("Alan Tull <atull@kernel.org>");
MODULE_DESCRIPTION("Intel Stratix 10 SOC FPGA Manager");
MODULE_LICENSE("GPL v2");