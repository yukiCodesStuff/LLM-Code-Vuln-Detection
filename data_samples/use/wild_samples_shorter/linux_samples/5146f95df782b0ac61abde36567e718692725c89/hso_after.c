		return -EIO;
	}

	/* check if we have a valid interface */
	if (if_num > 16) {
		kfree(config_data);
		return -EINVAL;
	}

	switch (config_data[if_num]) {
	case 0x0:
		result = 0;
		break;

	/* Get the interface/port specification from either driver_info or from
	 * the device itself */
	if (id->driver_info) {
		/* if_num is controlled by the device, driver_info is a 0 terminated
		 * array. Make sure, the access is in bounds! */
		for (i = 0; i <= if_num; ++i)
			if (((u32 *)(id->driver_info))[i] == 0)
				goto exit;
		port_spec = ((u32 *)(id->driver_info))[if_num];
	} else {
		port_spec = hso_get_config_data(interface);
		if (port_spec < 0)
			goto exit;
	}

	/* Check if we need to switch to alt interfaces prior to port
	 * configuration */
	if (interface->num_altsetting > 1)