		if (orig_fdt_size && fdt_totalsize(orig_fdt) > orig_fdt_size) {
			pr_efi_err(sys_table, "Truncated device tree! foo!\n");
			return EFI_LOAD_ERROR;
		}
	}

	if (orig_fdt)
		status = fdt_open_into(orig_fdt, fdt, new_fdt_size);
	else
		status = fdt_create_empty_tree(fdt, new_fdt_size);

	if (status != 0)
		goto fdt_set_fail;

	/*
	 * Delete any memory nodes present. We must delete nodes which
	 * early_init_dt_scan_memory may try to use.
	 */
	prev = 0;
	for (;;) {
		const char *type;
		int len;

		node = fdt_next_node(fdt, prev, NULL);
		if (node < 0)
			break;

		type = fdt_getprop(fdt, node, "device_type", &len);
		if (type && strncmp(type, "memory", len) == 0) {
			fdt_del_node(fdt, node);
			continue;
		}

		prev = node;
	}