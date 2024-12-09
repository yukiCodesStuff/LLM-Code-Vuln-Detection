	struct ni_power_info *ni_pi;
	struct si_power_info *si_pi;
	struct atom_clock_dividers dividers;
	enum pci_bus_speed speed_cap = PCI_SPEED_UNKNOWN;
	struct pci_dev *root = rdev->pdev->bus->self;
	int ret;

	si_pi = kzalloc(sizeof(struct si_power_info), GFP_KERNEL);
	eg_pi = &ni_pi->eg;
	pi = &eg_pi->rv7xx;

	if (!pci_is_root_bus(rdev->pdev->bus))
		speed_cap = pcie_get_speed_cap(root);
	if (speed_cap == PCI_SPEED_UNKNOWN) {
		si_pi->sys_pcie_mask = 0;
	} else {
		if (speed_cap == PCIE_SPEED_8_0GT)