	 * just how GSIs get registered.
	 */
	__acpi_register_gsi = acpi_register_gsi_xen_hvm;
#endif

#ifdef CONFIG_PCI_MSI
	/*
}

#ifdef CONFIG_XEN_DOM0
static __init void xen_setup_acpi_sci(void)
{
	int rc;
	int trigger, polarity;
	int gsi = acpi_sci_override_gsi;
	int irq = -1;
	int gsi_override = -1;

	if (!gsi)
		return;

	rc = acpi_get_override_irq(gsi, &trigger, &polarity);
	if (rc) {
		printk(KERN_WARNING "xen: acpi_get_override_irq failed for acpi"
				" sci, rc=%d\n", rc);
		return;
	}
	trigger = trigger ? ACPI_LEVEL_SENSITIVE : ACPI_EDGE_SENSITIVE;
	polarity = polarity ? ACPI_ACTIVE_LOW : ACPI_ACTIVE_HIGH;

	printk(KERN_INFO "xen: sci override: global_irq=%d trigger=%d "
			"polarity=%d\n", gsi, trigger, polarity);

	/* Before we bind the GSI to a Linux IRQ, check whether
	 * we need to override it with bus_irq (IRQ) value. Usually for
	 * IRQs below IRQ_LEGACY_IRQ this holds IRQ == GSI, as so:
	 *  ACPI: INT_SRC_OVR (bus 0 bus_irq 9 global_irq 9 low level)
	 * but there are oddballs where the IRQ != GSI:
	 *  ACPI: INT_SRC_OVR (bus 0 bus_irq 9 global_irq 20 low level)
	 * which ends up being: gsi_to_irq[9] == 20
	 * (which is what acpi_gsi_to_irq ends up calling when starting the
	 * the ACPI interpreter and keels over since IRQ 9 has not been
	 * setup as we had setup IRQ 20 for it).
	 */
	if (acpi_gsi_to_irq(gsi, &irq) == 0) {
		/* Use the provided value if it's valid. */
		if (irq >= 0)
			gsi_override = irq;
	}

	gsi = xen_register_gsi(gsi, gsi_override, trigger, polarity);
	printk(KERN_INFO "xen: acpi sci %d\n", gsi);

	return;
}

int __init pci_xen_initial_domain(void)
{
	int irq;

	x86_msi.restore_msi_irqs = xen_initdom_restore_msi_irqs;
	pci_msi_ignore_mask = 1;
#endif
	xen_setup_acpi_sci();
	__acpi_register_gsi = acpi_register_gsi_xen;
	/* Pre-allocate legacy irqs */
	for (irq = 0; irq < nr_legacy_irqs(); irq++) {
		int trigger, polarity;
