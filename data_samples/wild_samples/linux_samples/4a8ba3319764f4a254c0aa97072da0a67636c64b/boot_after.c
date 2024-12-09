	switch (trigger) {
	case 1:		/* Edge - clear */
		new &= ~mask;
		break;
	case 3:		/* Level - set */
		new |= mask;
		break;
	}

	if (old == new)
		return;

	printk(PREFIX "setting ELCR to %04x (from %04x)\n", new, old);
	outb(new, 0x4d0);
	outb(new >> 8, 0x4d1);
}

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
	if (cpu < 0) {
		pr_info(PREFIX "Unable to map lapic to logical cpu number\n");
		return cpu;
	}

	acpi_processor_set_pdc(handle);
	acpi_map_cpu2node(handle, cpu, physid);

	*pcpu = cpu;
	return 0;
}

/* wrapper to silence section mismatch warning */
int __ref acpi_map_cpu(acpi_handle handle, int physid, int *pcpu)
{
	return _acpi_map_lsapic(handle, physid, pcpu);
}
{
#ifdef CONFIG_ACPI_NUMA
	set_apicid_to_node(per_cpu(x86_cpu_to_apicid, cpu), NUMA_NO_NODE);
#endif

	per_cpu(x86_cpu_to_apicid, cpu) = -1;
	set_cpu_present(cpu, false);
	num_processors--;

	return (0);
}
EXPORT_SYMBOL(acpi_unmap_cpu);
#endif				/* CONFIG_ACPI_HOTPLUG_CPU */

int acpi_register_ioapic(acpi_handle handle, u64 phys_addr, u32 gsi_base)
{
	int ret = -ENOSYS;
#ifdef CONFIG_ACPI_HOTPLUG_IOAPIC
	int ioapic_id;
	u64 addr;
	struct ioapic_domain_cfg cfg = {
		.type = IOAPIC_DOMAIN_DYNAMIC,
		.ops = &acpi_irqdomain_ops,
	};

	ioapic_id = acpi_get_ioapic_id(handle, gsi_base, &addr);
	if (ioapic_id < 0) {
		unsigned long long uid;
		acpi_status status;

		status = acpi_evaluate_integer(handle, METHOD_NAME__UID,
					       NULL, &uid);
		if (ACPI_FAILURE(status)) {
			acpi_handle_warn(handle, "failed to get IOAPIC ID.\n");
			return -EINVAL;
		}
		ioapic_id = (int)uid;
	}

	mutex_lock(&acpi_ioapic_lock);
	ret  = mp_register_ioapic(ioapic_id, phys_addr, gsi_base, &cfg);
	mutex_unlock(&acpi_ioapic_lock);
#endif

	return ret;
}