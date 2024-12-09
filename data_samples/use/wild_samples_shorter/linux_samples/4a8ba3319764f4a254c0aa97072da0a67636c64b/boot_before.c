
int acpi_gsi_to_irq(u32 gsi, unsigned int *irqp)
{
	int irq;

	if (acpi_irq_model == ACPI_IRQ_MODEL_PIC) {
		*irqp = gsi;
	} else {
		mutex_lock(&acpi_ioapic_lock);
		irq = mp_map_gsi_to_irq(gsi,
					IOAPIC_MAP_ALLOC | IOAPIC_MAP_CHECK);
		mutex_unlock(&acpi_ioapic_lock);
		if (irq < 0)
			return -1;
		*irqp = irq;
	}
	return 0;
}
EXPORT_SYMBOL_GPL(acpi_gsi_to_irq);

int acpi_isa_irq_to_gsi(unsigned isa_irq, u32 *gsi)
}

/* wrapper to silence section mismatch warning */
int __ref acpi_map_lsapic(acpi_handle handle, int physid, int *pcpu)
{
	return _acpi_map_lsapic(handle, physid, pcpu);
}
EXPORT_SYMBOL(acpi_map_lsapic);

int acpi_unmap_lsapic(int cpu)
{
#ifdef CONFIG_ACPI_NUMA
	set_apicid_to_node(per_cpu(x86_cpu_to_apicid, cpu), NUMA_NO_NODE);
#endif

	return (0);
}

EXPORT_SYMBOL(acpi_unmap_lsapic);
#endif				/* CONFIG_ACPI_HOTPLUG_CPU */

int acpi_register_ioapic(acpi_handle handle, u64 phys_addr, u32 gsi_base)
{