{
	struct param_info *info = data;
	const void *prop;
	void *dest;
	u64 val;
	int i, len;

	if (depth != 1 ||
	    (strcmp(uname, "chosen") != 0 && strcmp(uname, "chosen@0") != 0))
		return 0;

	pr_info("Getting parameters from FDT:\n");

	for (i = 0; i < ARRAY_SIZE(dt_params); i++) {
		prop = of_get_flat_dt_prop(node, dt_params[i].propname, &len);
		if (!prop) {
			pr_err("Can't find %s in device tree!\n",
			       dt_params[i].name);
			return 0;
		}
		dest = info->params + dt_params[i].offset;

		val = of_read_number(prop, len / sizeof(u32));

		if (dt_params[i].size == sizeof(u32))
			*(u32 *)dest = val;
		else
			*(u64 *)dest = val;

		if (info->verbose)
			pr_info("  %s: 0x%0*llx\n", dt_params[i].name,
				dt_params[i].size * 2, val);
	}
	return 1;
}