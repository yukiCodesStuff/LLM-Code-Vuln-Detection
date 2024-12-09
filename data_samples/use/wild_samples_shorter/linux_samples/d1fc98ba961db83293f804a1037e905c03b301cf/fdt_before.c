	 */
	prev = 0;
	for (;;) {
		const char *type, *name;
		int len;

		node = fdt_next_node(fdt, prev, NULL);
		if (node < 0)