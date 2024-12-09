{
	cpumask_t tmp_map;
	int cpu;

	cpumask_complement(&tmp_map, cpu_present_mask);
	cpu = cpumask_first(&tmp_map);
	if (cpu >= nr_cpu_ids)
		return -EINVAL;

	acpi_map_cpu2node(handle, cpu, physid);

	set_cpu_present(cpu, true);
	ia64_cpu_to_sapicid[cpu] = physid;

	acpi_processor_set_pdc(handle);

	*pcpu = cpu;
	return (0);
}

/* wrapper to silence section mismatch warning */
int __ref acpi_map_cpu(acpi_handle handle, int physid, int *pcpu)
{
	return _acpi_map_lsapic(handle, physid, pcpu);
}
{
	ia64_cpu_to_sapicid[cpu] = -1;
	set_cpu_present(cpu, false);

#ifdef CONFIG_ACPI_NUMA
	/* NUMA specific cleanup's */
#endif

	return (0);
}
EXPORT_SYMBOL(acpi_unmap_cpu);
#endif				/* CONFIG_ACPI_HOTPLUG_CPU */

#ifdef CONFIG_ACPI_NUMA
static acpi_status acpi_map_iosapic(acpi_handle handle, u32 depth,
				    void *context, void **ret)
{
	struct acpi_buffer buffer = { ACPI_ALLOCATE_BUFFER, NULL };
	union acpi_object *obj;
	struct acpi_madt_io_sapic *iosapic;
	unsigned int gsi_base;
	int node;

	/* Only care about objects w/ a method that returns the MADT */
	if (ACPI_FAILURE(acpi_evaluate_object(handle, "_MAT", NULL, &buffer)))
		return AE_OK;

	if (!buffer.length || !buffer.pointer)
		return AE_OK;

	obj = buffer.pointer;
	if (obj->type != ACPI_TYPE_BUFFER ||
	    obj->buffer.length < sizeof(*iosapic)) {
		kfree(buffer.pointer);
		return AE_OK;
	}