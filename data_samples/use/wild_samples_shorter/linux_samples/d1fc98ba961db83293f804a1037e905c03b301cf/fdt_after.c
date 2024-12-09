	 */
	prev = 0;
	for (;;) {
		const char *type;
		int len;

		node = fdt_next_node(fdt, prev, NULL);
		if (node < 0)