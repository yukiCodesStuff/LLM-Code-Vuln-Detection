{
	struct cns3xxx_pcie *cnspci = pbus_to_cnspci(bus);
	int busno = bus->number;
	int slot = PCI_SLOT(devfn);
	void __iomem *base;

	/* If there is no link, just show the CNS PCI bridge. */
	if (!cnspci->linked && busno > 0)
		return NULL;

	/*
	 * The CNS PCI bridge doesn't fit into the PCI hierarchy, though
	 * we still want to access it.
	 * We place the host bridge on bus 0, and the directly connected
	 * device on bus 1, slot 0.
	 */
	if (busno == 0) { /* internal PCIe bus, host bridge device */
		if (devfn == 0) /* device# and function# are ignored by hw */
			base = cnspci->host_regs;
		else
			return NULL; /* no such device */

	} else if (busno == 1) { /* directly connected PCIe device */
		if (slot == 0) /* device# is ignored by hw */
			base = cnspci->cfg0_regs;
		else
			return NULL; /* no such device */
	} else /* remote PCI bus */
		base = cnspci->cfg1_regs + ((busno & 0xf) << 20);

	return base + (where & 0xffc) + (devfn << 12);
}

static int cns3xxx_pci_read_config(struct pci_bus *bus, unsigned int devfn,
				   int where, int size, u32 *val)
{
	int ret;
	u32 mask = (0x1ull << (size * 8)) - 1;
	int shift = (where % 4) * 8;

	ret = pci_generic_config_read32(bus, devfn, where, size, val);

	if (ret == PCIBIOS_SUCCESSFUL && !bus->number && !devfn &&
	    (where & 0xffc) == PCI_CLASS_REVISION)
		/*
		 * RC's class is 0xb, but Linux PCI driver needs 0x604
		 * for a PCIe bridge. So we must fixup the class code
		 * to 0x604 here.
		 */
		*val = ((((*val << shift) & 0xff) | (0x604 << 16)) >> shift) & mask;

	return ret;
}

static int cns3xxx_pci_setup(int nr, struct pci_sys_data *sys)
{
	struct cns3xxx_pcie *cnspci = sysdata_to_cnspci(sys);
	struct resource *res_io = &cnspci->res_io;
	struct resource *res_mem = &cnspci->res_mem;

	BUG_ON(request_resource(&iomem_resource, res_io) ||
	       request_resource(&iomem_resource, res_mem));

	pci_add_resource_offset(&sys->resources, res_io, sys->io_offset);
	pci_add_resource_offset(&sys->resources, res_mem, sys->mem_offset);

	return 1;
}

static struct pci_ops cns3xxx_pcie_ops = {
	.map_bus = cns3xxx_pci_map_bus,
	.read = cns3xxx_pci_read_config,
	.write = pci_generic_config_write,
};

static int cns3xxx_pcie_map_irq(const struct pci_dev *dev, u8 slot, u8 pin)
{
	struct cns3xxx_pcie *cnspci = pdev_to_cnspci(dev);
	int irq = cnspci->irqs[!!dev->bus->number];

	pr_info("PCIe map irq: %04d:%02x:%02x.%02x slot %d, pin %d, irq: %d\n",
		pci_domain_nr(dev->bus), dev->bus->number, PCI_SLOT(dev->devfn),
		PCI_FUNC(dev->devfn), slot, pin, irq);

	return irq;
}
{
	int ret;
	u32 mask = (0x1ull << (size * 8)) - 1;
	int shift = (where % 4) * 8;

	ret = pci_generic_config_read32(bus, devfn, where, size, val);

	if (ret == PCIBIOS_SUCCESSFUL && !bus->number && !devfn &&
	    (where & 0xffc) == PCI_CLASS_REVISION)
		/*
		 * RC's class is 0xb, but Linux PCI driver needs 0x604
		 * for a PCIe bridge. So we must fixup the class code
		 * to 0x604 here.
		 */
		*val = ((((*val << shift) & 0xff) | (0x604 << 16)) >> shift) & mask;

	return ret;
}

static int cns3xxx_pci_setup(int nr, struct pci_sys_data *sys)
{
	struct cns3xxx_pcie *cnspci = sysdata_to_cnspci(sys);
	struct resource *res_io = &cnspci->res_io;
	struct resource *res_mem = &cnspci->res_mem;

	BUG_ON(request_resource(&iomem_resource, res_io) ||
	       request_resource(&iomem_resource, res_mem));

	pci_add_resource_offset(&sys->resources, res_io, sys->io_offset);
	pci_add_resource_offset(&sys->resources, res_mem, sys->mem_offset);

	return 1;
}

static struct pci_ops cns3xxx_pcie_ops = {
	.map_bus = cns3xxx_pci_map_bus,
	.read = cns3xxx_pci_read_config,
	.write = pci_generic_config_write,
};

static int cns3xxx_pcie_map_irq(const struct pci_dev *dev, u8 slot, u8 pin)
{
	struct cns3xxx_pcie *cnspci = pdev_to_cnspci(dev);
	int irq = cnspci->irqs[!!dev->bus->number];

	pr_info("PCIe map irq: %04d:%02x:%02x.%02x slot %d, pin %d, irq: %d\n",
		pci_domain_nr(dev->bus), dev->bus->number, PCI_SLOT(dev->devfn),
		PCI_FUNC(dev->devfn), slot, pin, irq);

	return irq;
}