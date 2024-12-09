{
	if (!xen_have_vector_callback || !xen_feature(XENFEAT_hvm_pirqs))
		return 0;

#ifdef CONFIG_ACPI
	/*
	 * We don't want to change the actual ACPI delivery model,
	 * just how GSIs get registered.
	 */
	__acpi_register_gsi = acpi_register_gsi_xen_hvm;
	__acpi_unregister_gsi = NULL;
#endif

#ifdef CONFIG_PCI_MSI
	/*
	 * We need to wait until after x2apic is initialized
	 * before we can set MSI IRQ ops.
	 */
	x86_platform.apic_post_init = xen_msi_init;
#endif
	return 0;
}

#ifdef CONFIG_XEN_DOM0
int __init pci_xen_initial_domain(void)
{
	int irq;

#ifdef CONFIG_PCI_MSI
	x86_msi.setup_msi_irqs = xen_initdom_setup_msi_irqs;
	x86_msi.teardown_msi_irq = xen_teardown_msi_irq;
	x86_msi.restore_msi_irqs = xen_initdom_restore_msi_irqs;
	pci_msi_ignore_mask = 1;
#endif
	__acpi_register_gsi = acpi_register_gsi_xen;
	__acpi_unregister_gsi = NULL;
	/* Pre-allocate legacy irqs */
	for (irq = 0; irq < nr_legacy_irqs(); irq++) {
		int trigger, polarity;

		if (acpi_get_override_irq(irq, &trigger, &polarity) == -1)
			continue;

		xen_register_pirq(irq, -1 /* no GSI override */,
			trigger ? ACPI_LEVEL_SENSITIVE : ACPI_EDGE_SENSITIVE,
			true /* Map GSI to PIRQ */);
	}
{
	if (!xen_have_vector_callback || !xen_feature(XENFEAT_hvm_pirqs))
		return 0;

#ifdef CONFIG_ACPI
	/*
	 * We don't want to change the actual ACPI delivery model,
	 * just how GSIs get registered.
	 */
	__acpi_register_gsi = acpi_register_gsi_xen_hvm;
	__acpi_unregister_gsi = NULL;
#endif

#ifdef CONFIG_PCI_MSI
	/*
	 * We need to wait until after x2apic is initialized
	 * before we can set MSI IRQ ops.
	 */
	x86_platform.apic_post_init = xen_msi_init;
#endif
	return 0;
}

#ifdef CONFIG_XEN_DOM0
int __init pci_xen_initial_domain(void)
{
	int irq;

#ifdef CONFIG_PCI_MSI
	x86_msi.setup_msi_irqs = xen_initdom_setup_msi_irqs;
	x86_msi.teardown_msi_irq = xen_teardown_msi_irq;
	x86_msi.restore_msi_irqs = xen_initdom_restore_msi_irqs;
	pci_msi_ignore_mask = 1;
#endif
	__acpi_register_gsi = acpi_register_gsi_xen;
	__acpi_unregister_gsi = NULL;
	/* Pre-allocate legacy irqs */
	for (irq = 0; irq < nr_legacy_irqs(); irq++) {
		int trigger, polarity;

		if (acpi_get_override_irq(irq, &trigger, &polarity) == -1)
			continue;

		xen_register_pirq(irq, -1 /* no GSI override */,
			trigger ? ACPI_LEVEL_SENSITIVE : ACPI_EDGE_SENSITIVE,
			true /* Map GSI to PIRQ */);
	}
	if (0 == nr_ioapics) {
		for (irq = 0; irq < nr_legacy_irqs(); irq++)
			xen_bind_pirq_gsi_to_irq(irq, irq, 0, "xt-pic");
	}
	return 0;
}
{
	int irq;

#ifdef CONFIG_PCI_MSI
	x86_msi.setup_msi_irqs = xen_initdom_setup_msi_irqs;
	x86_msi.teardown_msi_irq = xen_teardown_msi_irq;
	x86_msi.restore_msi_irqs = xen_initdom_restore_msi_irqs;
	pci_msi_ignore_mask = 1;
#endif
	__acpi_register_gsi = acpi_register_gsi_xen;
	__acpi_unregister_gsi = NULL;
	/* Pre-allocate legacy irqs */
	for (irq = 0; irq < nr_legacy_irqs(); irq++) {
		int trigger, polarity;

		if (acpi_get_override_irq(irq, &trigger, &polarity) == -1)
			continue;

		xen_register_pirq(irq, -1 /* no GSI override */,
			trigger ? ACPI_LEVEL_SENSITIVE : ACPI_EDGE_SENSITIVE,
			true /* Map GSI to PIRQ */);
	}