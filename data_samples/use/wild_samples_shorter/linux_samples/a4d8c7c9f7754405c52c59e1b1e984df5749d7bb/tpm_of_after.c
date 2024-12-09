		goto cleanup_eio;
	}

	log->bios_event_log = kmalloc(*sizep, GFP_KERNEL);
	if (!log->bios_event_log) {
		pr_err("%s: ERROR - Not enough memory for BIOS measurements\n",
		       __func__);
		of_node_put(np);
		return -ENOMEM;
	}

	log->bios_event_log_end = log->bios_event_log + *sizep;

	memcpy(log->bios_event_log, __va(*basep), *sizep);
	of_node_put(np);

	return 0;

cleanup_eio: