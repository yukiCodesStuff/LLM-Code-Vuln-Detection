	if (last_invalid == 0) {
		printk(KERN_ERR FW_BUG PREFIX
		       "No valid BIOS _PSS frequency found for processor %d\n", pr->id);
		result = -EFAULT;
		kfree(pr->performance->states);
		pr->performance->states = NULL;
	}

	if (last_invalid > 0)
		pr->performance->state_count = last_invalid;

      end:
	kfree(buffer.pointer);

	return result;
}

int acpi_processor_get_performance_info(struct acpi_processor *pr)
{
	int result = 0;
	acpi_status status = AE_OK;
	acpi_handle handle = NULL;

	if (!pr || !pr->performance || !pr->handle)
		return -EINVAL;

	status = acpi_get_handle(pr->handle, "_PCT", &handle);
	if (ACPI_FAILURE(status)) {
		ACPI_DEBUG_PRINT((ACPI_DB_INFO,
				  "ACPI-based processor performance control unavailable\n"));
		return -ENODEV;
	}
	if (ACPI_SUCCESS(acpi_get_handle(pr->handle, "_PPC", &handle))){
		if(boot_cpu_has(X86_FEATURE_EST))
			printk(KERN_WARNING FW_BUG "BIOS needs update for CPU "
			       "frequency support\n");
	}
#endif
	return result;
}
EXPORT_SYMBOL_GPL(acpi_processor_get_performance_info);
int acpi_processor_notify_smm(struct module *calling_module)
{
	acpi_status status;
	static int is_done = 0;


	if (!(acpi_processor_ppc_status & PPC_REGISTERED))
		return -EBUSY;

	if (!try_module_get(calling_module))
		return -EINVAL;

	/* is_done is set to negative if an error occurred,
	 * and to postitive if _no_ error occurred, but SMM
	 * was already notified. This avoids double notification
	 * which might lead to unexpected results...
	 */
	if (is_done > 0) {
		module_put(calling_module);
		return 0;
	} else if (is_done < 0) {
		module_put(calling_module);
		return is_done;
	}

	is_done = -EIO;

	/* Can't write pstate_control to smi_command if either value is zero */
	if ((!acpi_gbl_FADT.smi_command) || (!acpi_gbl_FADT.pstate_control)) {
		ACPI_DEBUG_PRINT((ACPI_DB_INFO, "No SMI port or pstate_control\n"));
		module_put(calling_module);
		return 0;
	}

	ACPI_DEBUG_PRINT((ACPI_DB_INFO,
			  "Writing pstate_control [0x%x] to smi_command [0x%x]\n",
			  acpi_gbl_FADT.pstate_control, acpi_gbl_FADT.smi_command));

	status = acpi_os_write_port(acpi_gbl_FADT.smi_command,
				    (u32) acpi_gbl_FADT.pstate_control, 8);
	if (ACPI_FAILURE(status)) {
		ACPI_EXCEPTION((AE_INFO, status,
				"Failed to write pstate_control [0x%x] to "
				"smi_command [0x%x]", acpi_gbl_FADT.pstate_control,
				acpi_gbl_FADT.smi_command));
		module_put(calling_module);
		return status;
	}

	/* Success. If there's no _PPC, we need to fear nothing, so
	 * we can allow the cpufreq driver to be rmmod'ed. */
	is_done = 1;

	if (!(acpi_processor_ppc_status & PPC_IN_USE))
		module_put(calling_module);

	return 0;
}