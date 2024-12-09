
int acpi_gsi_to_irq(u32 gsi, unsigned int *irqp)
{
	int rc, irq, trigger, polarity;

	rc = acpi_get_override_irq(gsi, &trigger, &polarity);
	if (rc == 0) {
		trigger = trigger ? ACPI_LEVEL_SENSITIVE : ACPI_EDGE_SENSITIVE;
		polarity = polarity ? ACPI_ACTIVE_LOW : ACPI_ACTIVE_HIGH;
		irq = acpi_register_gsi(NULL, gsi, trigger, polarity);
		if (irq >= 0) {
			*irqp = irq;
			return 0;
		}
	}

	return -1;
}
EXPORT_SYMBOL_GPL(acpi_gsi_to_irq);

int acpi_isa_irq_to_gsi(unsigned isa_irq, u32 *gsi)
}

/* wrapper to silence section mismatch warning */
int __ref acpi_map_cpu(acpi_handle handle, int physid, int *pcpu)
{
	return _acpi_map_lsapic(handle, physid, pcpu);
}
EXPORT_SYMBOL(acpi_map_cpu);

int acpi_unmap_cpu(int cpu)
{
#ifdef CONFIG_ACPI_NUMA
	set_apicid_to_node(per_cpu(x86_cpu_to_apicid, cpu), NUMA_NO_NODE);
#endif

	return (0);
}
EXPORT_SYMBOL(acpi_unmap_cpu);
#endif				/* CONFIG_ACPI_HOTPLUG_CPU */

int acpi_register_ioapic(acpi_handle handle, u64 phys_addr, u32 gsi_base)
{