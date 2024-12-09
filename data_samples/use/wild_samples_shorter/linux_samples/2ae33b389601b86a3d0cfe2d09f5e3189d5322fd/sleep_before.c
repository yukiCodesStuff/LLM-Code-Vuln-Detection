		status = acpi_get_sleep_type_data(i, &type_a, &type_b);
		if (ACPI_SUCCESS(status)) {
			sleep_states[i] = 1;
			pr_cont(" S%d", i);
		}
	}

	suspend_set_ops(old_suspend_ordering ?
	hibernation_set_ops(old_suspend_ordering ?
			&acpi_hibernation_ops_old : &acpi_hibernation_ops);
	sleep_states[ACPI_STATE_S4] = 1;
	pr_cont(KERN_CONT " S4");
	if (nosigcheck)
		return;

	acpi_get_table(ACPI_SIG_FACS, 1, (struct acpi_table_header **)&facs);
{
	acpi_status status;
	u8 type_a, type_b;

	if (acpi_disabled)
		return 0;

	acpi_sleep_dmi_check();

	sleep_states[ACPI_STATE_S0] = 1;
	pr_info(PREFIX "(supports S0");

	acpi_sleep_suspend_setup();
	acpi_sleep_hibernate_setup();

	status = acpi_get_sleep_type_data(ACPI_STATE_S5, &type_a, &type_b);
	if (ACPI_SUCCESS(status)) {
		sleep_states[ACPI_STATE_S5] = 1;
		pr_cont(" S5");
		pm_power_off_prepare = acpi_power_off_prepare;
		pm_power_off = acpi_power_off;
	}
	pr_cont(")\n");
	/*
	 * Register the tts_notifier to reboot notifier list so that the _TTS
	 * object can also be evaluated when the system enters S5.
	 */