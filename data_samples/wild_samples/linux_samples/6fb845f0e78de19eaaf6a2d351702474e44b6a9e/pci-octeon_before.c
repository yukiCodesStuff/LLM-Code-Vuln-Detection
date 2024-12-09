{
	union cvmx_npi_mem_access_subidx mem_access;
	int index;

	/* Only these chips have PCI */
	if (octeon_has_feature(OCTEON_FEATURE_PCIE))
		return 0;

	/* Point pcibios_map_irq() to the PCI version of it */
	octeon_pcibios_map_irq = octeon_pci_pcibios_map_irq;

	/* Only use the big bars on chips that support it */
	if (OCTEON_IS_MODEL(OCTEON_CN31XX) ||
	    OCTEON_IS_MODEL(OCTEON_CN38XX_PASS2) ||
	    OCTEON_IS_MODEL(OCTEON_CN38XX_PASS1))
		octeon_dma_bar_type = OCTEON_DMA_BAR_TYPE_SMALL;
	else
		octeon_dma_bar_type = OCTEON_DMA_BAR_TYPE_BIG;

	if (!octeon_is_pci_host()) {
		pr_notice("Not in host mode, PCI Controller not initialized\n");
		return 0;
	}
{
	union cvmx_npi_mem_access_subidx mem_access;
	int index;

	/* Only these chips have PCI */
	if (octeon_has_feature(OCTEON_FEATURE_PCIE))
		return 0;

	/* Point pcibios_map_irq() to the PCI version of it */
	octeon_pcibios_map_irq = octeon_pci_pcibios_map_irq;

	/* Only use the big bars on chips that support it */
	if (OCTEON_IS_MODEL(OCTEON_CN31XX) ||
	    OCTEON_IS_MODEL(OCTEON_CN38XX_PASS2) ||
	    OCTEON_IS_MODEL(OCTEON_CN38XX_PASS1))
		octeon_dma_bar_type = OCTEON_DMA_BAR_TYPE_SMALL;
	else
		octeon_dma_bar_type = OCTEON_DMA_BAR_TYPE_BIG;

	if (!octeon_is_pci_host()) {
		pr_notice("Not in host mode, PCI Controller not initialized\n");
		return 0;
	}