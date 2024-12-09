enum {
	SNBEP_PCI_QPI_PORT0_FILTER,
	SNBEP_PCI_QPI_PORT1_FILTER,
	HSWEP_PCI_PCU_3,
};

static int snbep_qpi_hw_config(struct intel_uncore_box *box, struct perf_event *event)
{
{
	if (hswep_uncore_cbox.num_boxes > boot_cpu_data.x86_max_cores)
		hswep_uncore_cbox.num_boxes = boot_cpu_data.x86_max_cores;

	/* Detect 6-8 core systems with only two SBOXes */
	if (uncore_extra_pci_dev[0][HSWEP_PCI_PCU_3]) {
		u32 capid4;

		pci_read_config_dword(uncore_extra_pci_dev[0][HSWEP_PCI_PCU_3],
				      0x94, &capid4);
		if (((capid4 >> 6) & 0x3) == 0)
			hswep_uncore_sbox.num_boxes = 2;
	}

	uncore_msr_uncores = hswep_msr_uncores;
}

static struct intel_uncore_type hswep_uncore_ha = {
		.driver_data = UNCORE_PCI_DEV_DATA(UNCORE_EXTRA_PCI_DEV,
						   SNBEP_PCI_QPI_PORT1_FILTER),
	},
	{ /* PCU.3 (for Capability registers) */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x2fc0),
		.driver_data = UNCORE_PCI_DEV_DATA(UNCORE_EXTRA_PCI_DEV,
						   HSWEP_PCI_PCU_3),
	},
	{ /* end: all zeroes */ }
};

static struct pci_driver hswep_uncore_pci_driver = {