{
	int i;

	if (device->wakeup.flags.valid)
		acpi_power_resources_list_free(&device->wakeup.resources);

	if (!device->flags.power_manageable)
		return;

	for (i = ACPI_STATE_D0; i <= ACPI_STATE_D3_HOT; i++) {
		struct acpi_device_power_state *ps = &device->power.states[i];
		acpi_power_resources_list_free(&ps->resources);
	}
{
	u32 i;

	/* Presence of _PS0|_PR0 indicates 'power manageable' */
	if (!acpi_has_method(device->handle, "_PS0") &&
	    !acpi_has_method(device->handle, "_PR0"))
		return;

	device->flags.power_manageable = 1;

	/*
	 * Power Management Flags
	 */
	if (acpi_has_method(device->handle, "_PSC"))
		device->power.flags.explicit_get = 1;

	if (acpi_has_method(device->handle, "_IRC"))
		device->power.flags.inrush_current = 1;

	if (acpi_has_method(device->handle, "_DSW"))
		device->power.flags.dsw_present = 1;

	/*
	 * Enumerate supported power management states
	 */
	for (i = ACPI_STATE_D0; i <= ACPI_STATE_D3_HOT; i++)
		acpi_bus_init_power_state(device, i);

	INIT_LIST_HEAD(&device->power.states[ACPI_STATE_D3_COLD].resources);

	/* Set defaults for D0 and D3 states (always valid) */
	device->power.states[ACPI_STATE_D0].flags.valid = 1;
	device->power.states[ACPI_STATE_D0].power = 100;
	device->power.states[ACPI_STATE_D3_COLD].flags.valid = 1;
	device->power.states[ACPI_STATE_D3_COLD].power = 0;

	/* Set D3cold's explicit_set flag if _PS3 exists. */
	if (device->power.states[ACPI_STATE_D3_HOT].flags.explicit_set)
		device->power.states[ACPI_STATE_D3_COLD].flags.explicit_set = 1;

	/* Presence of _PS3 or _PRx means we can put the device into D3 cold */
	if (device->power.states[ACPI_STATE_D3_HOT].flags.explicit_set ||
			device->power.flags.power_resources)
		device->power.states[ACPI_STATE_D3_COLD].flags.os_accessible = 1;

	if (acpi_bus_init_power(device)) {
		acpi_free_power_resources_lists(device);
		device->flags.power_manageable = 0;
	}
	if (!acpi_device_is_present(device)) {
		device->flags.visited = false;
		return;
	}