	if (octeon_has_feature(OCTEON_FEATURE_PCIE))
		return 0;

	/* Point pcibios_map_irq() to the PCI version of it */
	octeon_pcibios_map_irq = octeon_pci_pcibios_map_irq;

	/* Only use the big bars on chips that support it */
	else
		octeon_dma_bar_type = OCTEON_DMA_BAR_TYPE_BIG;

	if (!octeon_is_pci_host()) {
		pr_notice("Not in host mode, PCI Controller not initialized\n");
		return 0;
	}

	/* PCI I/O and PCI MEM values */
	set_io_port_base(OCTEON_PCI_IOSPACE_BASE);
	ioport_resource.start = 0;
	ioport_resource.end = OCTEON_PCI_IOSPACE_SIZE - 1;