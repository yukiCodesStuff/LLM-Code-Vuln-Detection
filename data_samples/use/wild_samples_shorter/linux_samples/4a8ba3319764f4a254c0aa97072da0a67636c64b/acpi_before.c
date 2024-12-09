}

/* wrapper to silence section mismatch warning */
int __ref acpi_map_lsapic(acpi_handle handle, int physid, int *pcpu)
{
	return _acpi_map_lsapic(handle, physid, pcpu);
}
EXPORT_SYMBOL(acpi_map_lsapic);

int acpi_unmap_lsapic(int cpu)
{
	ia64_cpu_to_sapicid[cpu] = -1;
	set_cpu_present(cpu, false);


	return (0);
}

EXPORT_SYMBOL(acpi_unmap_lsapic);
#endif				/* CONFIG_ACPI_HOTPLUG_CPU */

#ifdef CONFIG_ACPI_NUMA
static acpi_status acpi_map_iosapic(acpi_handle handle, u32 depth,