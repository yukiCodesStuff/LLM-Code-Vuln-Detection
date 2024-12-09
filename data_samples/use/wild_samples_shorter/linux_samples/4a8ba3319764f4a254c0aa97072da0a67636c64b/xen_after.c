	 * just how GSIs get registered.
	 */
	__acpi_register_gsi = acpi_register_gsi_xen_hvm;
	__acpi_unregister_gsi = NULL;
#endif

#ifdef CONFIG_PCI_MSI
	/*
}

#ifdef CONFIG_XEN_DOM0
int __init pci_xen_initial_domain(void)
{
	int irq;

	x86_msi.restore_msi_irqs = xen_initdom_restore_msi_irqs;
	pci_msi_ignore_mask = 1;
#endif
	__acpi_register_gsi = acpi_register_gsi_xen;
	__acpi_unregister_gsi = NULL;
	/* Pre-allocate legacy irqs */
	for (irq = 0; irq < nr_legacy_irqs(); irq++) {
		int trigger, polarity;
