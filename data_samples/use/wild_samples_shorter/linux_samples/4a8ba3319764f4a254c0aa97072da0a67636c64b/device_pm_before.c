
	device->power.state = ACPI_STATE_UNKNOWN;
	if (!acpi_device_is_present(device))
		return 0;

	result = acpi_device_get_power(device, &state);
	if (result)
		return result;