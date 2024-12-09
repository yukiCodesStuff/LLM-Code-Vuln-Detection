	u16 data_offset, size;
	u8 frev, crev;
	struct ci_power_info *pi;
	enum pci_bus_speed speed_cap;
	struct pci_dev *root = rdev->pdev->bus->self;
	int ret;

	pi = kzalloc(sizeof(struct ci_power_info), GFP_KERNEL);
		return -ENOMEM;
	rdev->pm.dpm.priv = pi;

	speed_cap = pcie_get_speed_cap(root);
	if (speed_cap == PCI_SPEED_UNKNOWN) {
		pi->sys_pcie_mask = 0;
	} else {
		if (speed_cap == PCIE_SPEED_8_0GT)