		if (ACPI_SUCCESS(status)) {
			sleep_states[i] = 1;
		}
{
	acpi_status status;
	u8 type_a, type_b;

	status = acpi_get_sleep_type_data(ACPI_STATE_S4, &type_a, &type_b);
	if (ACPI_FAILURE(status))
		return;

	hibernation_set_ops(old_suspend_ordering ?
			&acpi_hibernation_ops_old : &acpi_hibernation_ops);
	sleep_states[ACPI_STATE_S4] = 1;
	if (nosigcheck)
		return;

	acpi_get_table(ACPI_SIG_FACS, 1, (struct acpi_table_header **)&facs);
	if (facs)
		s4_hardware_signature = facs->hardware_signature;
}
#else /* !CONFIG_HIBERNATION */
static inline void acpi_sleep_hibernate_setup(void) {}
#endif /* !CONFIG_HIBERNATION */

int acpi_suspend(u32 acpi_state)
{
	suspend_state_t states[] = {
		[1] = PM_SUSPEND_STANDBY,
		[3] = PM_SUSPEND_MEM,
		[5] = PM_SUSPEND_MAX
	};

	if (acpi_state < 6 && states[acpi_state])
		return pm_suspend(states[acpi_state]);
	if (acpi_state == 4)
		return hibernate();
	return -EINVAL;
}

static void acpi_power_off_prepare(void)
{
	/* Prepare to power off the system */
	acpi_sleep_prepare(ACPI_STATE_S5);
	acpi_disable_all_gpes();
}

static void acpi_power_off(void)
{
	/* acpi_sleep_prepare(ACPI_STATE_S5) should have already been called */
	printk(KERN_DEBUG "%s called\n", __func__);
	local_irq_disable();
	acpi_enter_sleep_state(ACPI_STATE_S5);
}

int __init acpi_sleep_init(void)
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