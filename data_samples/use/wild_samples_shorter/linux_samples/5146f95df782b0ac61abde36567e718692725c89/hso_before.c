		return -EIO;
	}

	switch (config_data[if_num]) {
	case 0x0:
		result = 0;
		break;

	/* Get the interface/port specification from either driver_info or from
	 * the device itself */
	if (id->driver_info)
		port_spec = ((u32 *)(id->driver_info))[if_num];
	else
		port_spec = hso_get_config_data(interface);

	/* Check if we need to switch to alt interfaces prior to port
	 * configuration */
	if (interface->num_altsetting > 1)