{
	union cvmx_npi_mem_access_subidx mem_access;
	int index;

	/* Only these chips have PCI */
	if (octeon_has_feature(OCTEON_FEATURE_PCIE))
		return 0;

	if (!octeon_is_pci_host()) {
		pr_notice("Not in host mode, PCI Controller not initialized\n");
		return 0;
	}
	if (!octeon_is_pci_host()) {
		pr_notice("Not in host mode, PCI Controller not initialized\n");
		return 0;
	}

	/* Point pcibios_map_irq() to the PCI version of it */
	octeon_pcibios_map_irq = octeon_pci_pcibios_map_irq;

	/* Only use the big bars on chips that support it */
	if (OCTEON_IS_MODEL(OCTEON_CN31XX) ||
	    OCTEON_IS_MODEL(OCTEON_CN38XX_PASS2) ||
	    OCTEON_IS_MODEL(OCTEON_CN38XX_PASS1))
		octeon_dma_bar_type = OCTEON_DMA_BAR_TYPE_SMALL;
	else
		octeon_dma_bar_type = OCTEON_DMA_BAR_TYPE_BIG;

	/* PCI I/O and PCI MEM values */
	set_io_port_base(OCTEON_PCI_IOSPACE_BASE);
	ioport_resource.start = 0;
	ioport_resource.end = OCTEON_PCI_IOSPACE_SIZE - 1;

	pr_notice("%s Octeon big bar support\n",
		  (octeon_dma_bar_type ==
		  OCTEON_DMA_BAR_TYPE_BIG) ? "Enabling" : "Disabling");

	octeon_pci_initialize();

	mem_access.u64 = 0;
	mem_access.s.esr = 1;	/* Endian-Swap on read. */
	mem_access.s.esw = 1;	/* Endian-Swap on write. */
	mem_access.s.nsr = 0;	/* No-Snoop on read. */
	mem_access.s.nsw = 0;	/* No-Snoop on write. */
	mem_access.s.ror = 0;	/* Relax Read on read. */
	mem_access.s.row = 0;	/* Relax Order on write. */
	mem_access.s.ba = 0;	/* PCI Address bits [63:36]. */
	cvmx_write_csr(CVMX_NPI_MEM_ACCESS_SUBID3, mem_access.u64);

	/*
	 * Remap the Octeon BAR 2 above all 32 bit devices
	 * (0x8000000000ul).  This is done here so it is remapped
	 * before the readl()'s below. We don't want BAR2 overlapping
	 * with BAR0/BAR1 during these reads.
	 */
	octeon_npi_write32(CVMX_NPI_PCI_CFG08,
			   (u32)(OCTEON_BAR2_PCI_ADDRESS & 0xffffffffull));
	octeon_npi_write32(CVMX_NPI_PCI_CFG09,
			   (u32)(OCTEON_BAR2_PCI_ADDRESS >> 32));

	if (octeon_dma_bar_type == OCTEON_DMA_BAR_TYPE_BIG) {
		/* Remap the Octeon BAR 0 to 0-2GB */
		octeon_npi_write32(CVMX_NPI_PCI_CFG04, 0);
		octeon_npi_write32(CVMX_NPI_PCI_CFG05, 0);

		/*
		 * Remap the Octeon BAR 1 to map 2GB-4GB (minus the
		 * BAR 1 hole).
		 */
		octeon_npi_write32(CVMX_NPI_PCI_CFG06, 2ul << 30);
		octeon_npi_write32(CVMX_NPI_PCI_CFG07, 0);

		/* BAR1 movable mappings set for identity mapping */
		octeon_bar1_pci_phys = 0x80000000ull;
		for (index = 0; index < 32; index++) {
			union cvmx_pci_bar1_indexx bar1_index;

			bar1_index.u32 = 0;
			/* Address bits[35:22] sent to L2C */
			bar1_index.s.addr_idx =
				(octeon_bar1_pci_phys >> 22) + index;
			/* Don't put PCI accesses in L2. */
			bar1_index.s.ca = 1;
			/* Endian Swap Mode */
			bar1_index.s.end_swp = 1;
			/* Set '1' when the selected address range is valid. */
			bar1_index.s.addr_v = 1;
			octeon_npi_write32(CVMX_NPI_PCI_BAR1_INDEXX(index),
					   bar1_index.u32);
		}

		/* Devices go after BAR1 */
		octeon_pci_mem_resource.start =
			OCTEON_PCI_MEMSPACE_OFFSET + (4ul << 30) -
			(OCTEON_PCI_BAR1_HOLE_SIZE << 20);
		octeon_pci_mem_resource.end =
			octeon_pci_mem_resource.start + (1ul << 30);
	} else {
		/* Remap the Octeon BAR 0 to map 128MB-(128MB+4KB) */
		octeon_npi_write32(CVMX_NPI_PCI_CFG04, 128ul << 20);
		octeon_npi_write32(CVMX_NPI_PCI_CFG05, 0);

		/* Remap the Octeon BAR 1 to map 0-128MB */
		octeon_npi_write32(CVMX_NPI_PCI_CFG06, 0);
		octeon_npi_write32(CVMX_NPI_PCI_CFG07, 0);

		/* BAR1 movable regions contiguous to cover the swiotlb */
		octeon_bar1_pci_phys =
			virt_to_phys(octeon_swiotlb) & ~((1ull << 22) - 1);

		for (index = 0; index < 32; index++) {
			union cvmx_pci_bar1_indexx bar1_index;

			bar1_index.u32 = 0;
			/* Address bits[35:22] sent to L2C */
			bar1_index.s.addr_idx =
				(octeon_bar1_pci_phys >> 22) + index;
			/* Don't put PCI accesses in L2. */
			bar1_index.s.ca = 1;
			/* Endian Swap Mode */
			bar1_index.s.end_swp = 1;
			/* Set '1' when the selected address range is valid. */
			bar1_index.s.addr_v = 1;
			octeon_npi_write32(CVMX_NPI_PCI_BAR1_INDEXX(index),
					   bar1_index.u32);
		}

		/* Devices go after BAR0 */
		octeon_pci_mem_resource.start =
			OCTEON_PCI_MEMSPACE_OFFSET + (128ul << 20) +
			(4ul << 10);
		octeon_pci_mem_resource.end =
			octeon_pci_mem_resource.start + (1ul << 30);
	}

	register_pci_controller(&octeon_pci_controller);

	/*
	 * Clear any errors that might be pending from before the bus
	 * was setup properly.
	 */
	cvmx_write_csr(CVMX_NPI_PCI_INT_SUM2, -1);

	if (IS_ERR(platform_device_register_simple("octeon_pci_edac",
						   -1, NULL, 0)))
		pr_err("Registration of co_pci_edac failed!\n");

	octeon_pci_dma_init();

	return 0;
}