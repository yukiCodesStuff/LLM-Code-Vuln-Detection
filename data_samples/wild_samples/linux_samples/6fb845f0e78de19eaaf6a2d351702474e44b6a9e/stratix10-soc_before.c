{
	struct device_node *fw_np;
	struct device_node *np;
	int ret;

	fw_np = of_find_node_by_name(NULL, "svc");
	if (!fw_np)
		return -ENODEV;

	np = of_find_matching_node(fw_np, s10_of_match);
	if (!np) {
		of_node_put(fw_np);
		return -ENODEV;
	}