	return result;
}

int acpi_processor_get_performance_info(struct acpi_processor *pr)
{
	int result = 0;
	acpi_status status = AE_OK;
	acpi_handle handle = NULL;
#endif
	return result;
}
EXPORT_SYMBOL_GPL(acpi_processor_get_performance_info);
int acpi_processor_notify_smm(struct module *calling_module)
{
	acpi_status status;
	static int is_done = 0;