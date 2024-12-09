	if (device->wakeup.flags.valid)
		acpi_power_resources_list_free(&device->wakeup.resources);

	if (!device->flags.power_manageable)
		return;

	for (i = ACPI_STATE_D0; i <= ACPI_STATE_D3_HOT; i++) {
		struct acpi_device_power_state *ps = &device->power.states[i];
			device->power.flags.power_resources)
		device->power.states[ACPI_STATE_D3_COLD].flags.os_accessible = 1;

	if (acpi_bus_init_power(device)) {
		acpi_free_power_resources_lists(device);
		device->flags.power_manageable = 0;
	}
}

static void acpi_bus_get_flags(struct acpi_device *device)
{
	/* Skip devices that are not present. */
	if (!acpi_device_is_present(device)) {
		device->flags.visited = false;
		return;
	}
	if (device->handler)
		goto ok;

	if (!device->flags.initialized) {
		acpi_bus_update_power(device, NULL);
		device->flags.initialized = true;
	}
	device->flags.visited = false;
	ret = acpi_scan_attach_handler(device);