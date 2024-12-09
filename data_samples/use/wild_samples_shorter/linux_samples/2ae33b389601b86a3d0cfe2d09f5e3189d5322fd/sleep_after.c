		status = acpi_get_sleep_type_data(i, &type_a, &type_b);
		if (ACPI_SUCCESS(status)) {
			sleep_states[i] = 1;
		}
	}

	suspend_set_ops(old_suspend_ordering ?
	hibernation_set_ops(old_suspend_ordering ?
			&acpi_hibernation_ops_old : &acpi_hibernation_ops);
	sleep_states[ACPI_STATE_S4] = 1;
	if (nosigcheck)
		return;

	acpi_get_table(ACPI_SIG_FACS, 1, (struct acpi_table_header **)&facs);
{
	acpi_status status;
	u8 type_a, type_b;
	char supported[ACPI_S_STATE_COUNT * 3 + 1];
	char *pos = supported;
	int i;

	if (acpi_disabled)
		return 0;

	acpi_sleep_dmi_check();

	sleep_states[ACPI_STATE_S0] = 1;

	acpi_sleep_suspend_setup();
	acpi_sleep_hibernate_setup();

	status = acpi_get_sleep_type_data(ACPI_STATE_S5, &type_a, &type_b);
	if (ACPI_SUCCESS(status)) {
		sleep_states[ACPI_STATE_S5] = 1;
		pm_power_off_prepare = acpi_power_off_prepare;
		pm_power_off = acpi_power_off;
	}

	supported[0] = 0;
	for (i = 0; i < ACPI_S_STATE_COUNT; i++) {
		if (sleep_states[i])
			pos += sprintf(pos, " S%d", i);
	}
	pr_info(PREFIX "(supports%s)\n", supported);

	/*
	 * Register the tts_notifier to reboot notifier list so that the _TTS
	 * object can also be evaluated when the system enters S5.
	 */