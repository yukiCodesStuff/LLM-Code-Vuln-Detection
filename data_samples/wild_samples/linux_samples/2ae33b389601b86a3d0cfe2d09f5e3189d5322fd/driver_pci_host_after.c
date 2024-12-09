	if (!pc_host)  {
		bcma_err(bus, "can not allocate memory");
		return;
	}

	spin_lock_init(&pc_host->cfgspace_lock);

	pc->host_controller = pc_host;
	pc_host->pci_controller.io_resource = &pc_host->io_resource;
	pc_host->pci_controller.mem_resource = &pc_host->mem_resource;
	pc_host->pci_controller.pci_ops = &pc_host->pci_ops;
	pc_host->pdev = pc;

	pci_membase_1G = BCMA_SOC_PCI_DMA;
	pc_host->host_cfg_addr = BCMA_SOC_PCI_CFG;

	pc_host->pci_ops.read = bcma_core_pci_hostmode_read_config;
	pc_host->pci_ops.write = bcma_core_pci_hostmode_write_config;

	pc_host->mem_resource.name = "BCMA PCIcore external memory",
	pc_host->mem_resource.start = BCMA_SOC_PCI_DMA;
	pc_host->mem_resource.end = BCMA_SOC_PCI_DMA + BCMA_SOC_PCI_DMA_SZ - 1;
	pc_host->mem_resource.flags = IORESOURCE_MEM | IORESOURCE_PCI_FIXED;

	pc_host->io_resource.name = "BCMA PCIcore external I/O",
	pc_host->io_resource.start = 0x100;
	pc_host->io_resource.end = 0x7FF;
	pc_host->io_resource.flags = IORESOURCE_IO | IORESOURCE_PCI_FIXED;

	/* Reset RC */
	usleep_range(3000, 5000);
	pcicore_write32(pc, BCMA_CORE_PCI_CTL, BCMA_CORE_PCI_CTL_RST_OE);
	msleep(50);
	pcicore_write32(pc, BCMA_CORE_PCI_CTL, BCMA_CORE_PCI_CTL_RST |
			BCMA_CORE_PCI_CTL_RST_OE);

	/* 64 MB I/O access window. On 4716, use
	 * sbtopcie0 to access the device registers. We
	 * can't use address match 2 (1 GB window) region
	 * as mips can't generate 64-bit address on the
	 * backplane.
	 */
	if (bus->chipinfo.id == BCMA_CHIP_ID_BCM4716 ||
	    bus->chipinfo.id == BCMA_CHIP_ID_BCM4748) {
		pc_host->mem_resource.start = BCMA_SOC_PCI_MEM;
		pc_host->mem_resource.end = BCMA_SOC_PCI_MEM +
					    BCMA_SOC_PCI_MEM_SZ - 1;
		pcicore_write32(pc, BCMA_CORE_PCI_SBTOPCI0,
				BCMA_CORE_PCI_SBTOPCI_MEM | BCMA_SOC_PCI_MEM);
	} else if (bus->chipinfo.id == BCMA_CHIP_ID_BCM4706) {
		tmp = BCMA_CORE_PCI_SBTOPCI_MEM;
		tmp |= BCMA_CORE_PCI_SBTOPCI_PREF;
		tmp |= BCMA_CORE_PCI_SBTOPCI_BURST;
		if (pc->core->core_unit == 0) {
			pc_host->mem_resource.start = BCMA_SOC_PCI_MEM;
			pc_host->mem_resource.end = BCMA_SOC_PCI_MEM +
						    BCMA_SOC_PCI_MEM_SZ - 1;
			pc_host->io_resource.start = 0x100;
			pc_host->io_resource.end = 0x47F;
			pci_membase_1G = BCMA_SOC_PCIE_DMA_H32;
			pcicore_write32(pc, BCMA_CORE_PCI_SBTOPCI0,
					tmp | BCMA_SOC_PCI_MEM);
		} else if (pc->core->core_unit == 1) {
			pc_host->mem_resource.start = BCMA_SOC_PCI1_MEM;
			pc_host->mem_resource.end = BCMA_SOC_PCI1_MEM +
						    BCMA_SOC_PCI_MEM_SZ - 1;
			pc_host->io_resource.start = 0x480;
			pc_host->io_resource.end = 0x7FF;
			pci_membase_1G = BCMA_SOC_PCIE1_DMA_H32;
			pc_host->host_cfg_addr = BCMA_SOC_PCI1_CFG;
			pcicore_write32(pc, BCMA_CORE_PCI_SBTOPCI0,
					tmp | BCMA_SOC_PCI1_MEM);
		}
	} else
		pcicore_write32(pc, BCMA_CORE_PCI_SBTOPCI0,
				BCMA_CORE_PCI_SBTOPCI_IO);

	/* 64 MB configuration access window */
	pcicore_write32(pc, BCMA_CORE_PCI_SBTOPCI1, BCMA_CORE_PCI_SBTOPCI_CFG0);

	/* 1 GB memory access window */
	pcicore_write32(pc, BCMA_CORE_PCI_SBTOPCI2,
			BCMA_CORE_PCI_SBTOPCI_MEM | pci_membase_1G);


	/* As per PCI Express Base Spec 1.1 we need to wait for
	 * at least 100 ms from the end of a reset (cold/warm/hot)
	 * before issuing configuration requests to PCI Express
	 * devices.
	 */
	msleep(100);

	bcma_core_pci_enable_crs(pc);

	if (bus->chipinfo.id == BCMA_CHIP_ID_BCM4706 ||
	    bus->chipinfo.id == BCMA_CHIP_ID_BCM4716) {
		u16 val16;
		bcma_extpci_read_config(pc, 0, 0, BCMA_CORE_PCI_CFG_DEVCTRL,
					&val16, sizeof(val16));
		val16 |= (2 << 5);	/* Max payload size of 512 */
		val16 |= (2 << 12);	/* MRRS 512 */
		bcma_extpci_write_config(pc, 0, 0, BCMA_CORE_PCI_CFG_DEVCTRL,
					 &val16, sizeof(val16));
	}

	/* Enable PCI bridge BAR0 memory & master access */
	tmp = PCI_COMMAND_MASTER | PCI_COMMAND_MEMORY;
	bcma_extpci_write_config(pc, 0, 0, PCI_COMMAND, &tmp, sizeof(tmp));

	/* Enable PCI interrupts */
	pcicore_write32(pc, BCMA_CORE_PCI_IMASK, BCMA_CORE_PCI_IMASK_INTA);

	/* Ok, ready to run, register it to the system.
	 * The following needs change, if we want to port hostmode
	 * to non-MIPS platform. */
	io_map_base = (unsigned long)ioremap_nocache(pc_host->mem_resource.start,
						     resource_size(&pc_host->mem_resource));
	pc_host->pci_controller.io_map_base = io_map_base;
	set_io_port_base(pc_host->pci_controller.io_map_base);
	/* Give some time to the PCI controller to configure itself with the new
	 * values. Not waiting at this point causes crashes of the machine. */
	usleep_range(10000, 15000);
	register_pci_controller(&pc_host->pci_controller);
	return;
}

/* Early PCI fixup for a device on the PCI-core bridge. */
static void bcma_core_pci_fixup_pcibridge(struct pci_dev *dev)
{
	if (dev->bus->ops->read != bcma_core_pci_hostmode_read_config) {
		/* This is not a device on the PCI-core bridge. */
		return;
	}
	if (PCI_SLOT(dev->devfn) != 0)
		return;

	pr_info("PCI: Fixing up bridge %s\n", pci_name(dev));

	/* Enable PCI bridge bus mastering and memory space */
	pci_set_master(dev);
	if (pcibios_enable_device(dev, ~0) < 0) {
		pr_err("PCI: BCMA bridge enable failed\n");
		return;
	}

	/* Enable PCI bridge BAR1 prefetch and burst */
	pci_write_config_dword(dev, BCMA_PCI_BAR1_CONTROL, 3);
}
DECLARE_PCI_FIXUP_EARLY(PCI_ANY_ID, PCI_ANY_ID, bcma_core_pci_fixup_pcibridge);

/* Early PCI fixup for all PCI-cores to set the correct memory address. */
static void bcma_core_pci_fixup_addresses(struct pci_dev *dev)
{
	struct resource *res;
	int pos, err;

	if (dev->bus->ops->read != bcma_core_pci_hostmode_read_config) {
		/* This is not a device on the PCI-core bridge. */
		return;
	}
	if (PCI_SLOT(dev->devfn) == 0)
		return;

	pr_info("PCI: Fixing up addresses %s\n", pci_name(dev));

	for (pos = 0; pos < 6; pos++) {
		res = &dev->resource[pos];
		if (res->flags & (IORESOURCE_IO | IORESOURCE_MEM)) {
			err = pci_assign_resource(dev, pos);
			if (err)
				pr_err("PCI: Problem fixing up the addresses on %s\n",
				       pci_name(dev));
		}
	}
}
DECLARE_PCI_FIXUP_HEADER(PCI_ANY_ID, PCI_ANY_ID, bcma_core_pci_fixup_addresses);

/* This function is called when doing a pci_enable_device().
 * We must first check if the device is a device on the PCI-core bridge. */
int bcma_core_pci_plat_dev_init(struct pci_dev *dev)
{
	struct bcma_drv_pci_host *pc_host;

	if (dev->bus->ops->read != bcma_core_pci_hostmode_read_config) {
		/* This is not a device on the PCI-core bridge. */
		return -ENODEV;
	}
	pc_host = container_of(dev->bus->ops, struct bcma_drv_pci_host,
			       pci_ops);

	pr_info("PCI: Fixing up device %s\n", pci_name(dev));

	/* Fix up interrupt lines */
	dev->irq = bcma_core_irq(pc_host->pdev->core);
	pci_write_config_byte(dev, PCI_INTERRUPT_LINE, dev->irq);

	return 0;
}
EXPORT_SYMBOL(bcma_core_pci_plat_dev_init);

/* PCI device IRQ mapping. */
int bcma_core_pci_pcibios_map_irq(const struct pci_dev *dev)
{
	struct bcma_drv_pci_host *pc_host;

	if (dev->bus->ops->read != bcma_core_pci_hostmode_read_config) {
		/* This is not a device on the PCI-core bridge. */
		return -ENODEV;
	}

	pc_host = container_of(dev->bus->ops, struct bcma_drv_pci_host,
			       pci_ops);
	return bcma_core_irq(pc_host->pdev->core);
}
EXPORT_SYMBOL(bcma_core_pci_pcibios_map_irq);