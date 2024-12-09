		for (i = 0; i < PCI_NUM_RESOURCES; i++) {
			struct resource *r = &dev->resource[i];

			if (r->parent || !r->start || !r->flags)
				continue;

			if (ofpci_verbose)
				printk("PCI: Claiming %s: "
				       "Resource %d: %016llx..%016llx [%x]\n",
				       pci_name(dev), i,
				       (unsigned long long)r->start,
				       (unsigned long long)r->end,
				       (unsigned int)r->flags);

			pci_claim_resource(dev, i);
		}
	}

	list_for_each_entry(child_bus, &bus->children, node)
		pci_claim_bus_resources(child_bus);
}

struct pci_bus *pci_scan_one_pbm(struct pci_pbm_info *pbm,
				 struct device *parent)
{
	LIST_HEAD(resources);
	struct device_node *node = pbm->op->dev.of_node;
	struct pci_bus *bus;

	printk("PCI: Scanning PBM %s\n", node->full_name);

	pci_add_resource_offset(&resources, &pbm->io_space,
				pbm->io_space.start);
	pci_add_resource_offset(&resources, &pbm->mem_space,
				pbm->mem_space.start);
	pbm->busn.start = pbm->pci_first_busno;
	pbm->busn.end	= pbm->pci_last_busno;
	pbm->busn.flags	= IORESOURCE_BUS;
	pci_add_resource(&resources, &pbm->busn);
	bus = pci_create_root_bus(parent, pbm->pci_first_busno, pbm->pci_ops,
				  pbm, &resources);
	if (!bus) {
		printk(KERN_ERR "Failed to create bus for %s\n",
		       node->full_name);
		pci_free_resource_list(&resources);
		return NULL;
	}

	pci_of_scan_bus(pbm, node, bus);
	pci_bus_add_devices(bus);
	pci_bus_register_of_sysfs(bus);

	pci_claim_bus_resources(bus);

	return bus;
}

void pcibios_fixup_bus(struct pci_bus *pbus)
{
}

resource_size_t pcibios_align_resource(void *data, const struct resource *res,
				resource_size_t size, resource_size_t align)
{
	return res->start;
}

int pcibios_enable_device(struct pci_dev *dev, int mask)
{
	u16 cmd, oldcmd;
	int i;

	pci_read_config_word(dev, PCI_COMMAND, &cmd);
	oldcmd = cmd;

	for (i = 0; i < PCI_NUM_RESOURCES; i++) {
		struct resource *res = &dev->resource[i];

		/* Only set up the requested stuff */
		if (!(mask & (1<<i)))
			continue;

		if (res->flags & IORESOURCE_IO)
			cmd |= PCI_COMMAND_IO;
		if (res->flags & IORESOURCE_MEM)
			cmd |= PCI_COMMAND_MEMORY;
	}

	if (cmd != oldcmd) {
		printk(KERN_DEBUG "PCI: Enabling device: (%s), cmd %x\n",
		       pci_name(dev), cmd);
                /* Enable the appropriate bits in the PCI command register.  */
		pci_write_config_word(dev, PCI_COMMAND, cmd);
	}
	return 0;
}

/* Platform support for /proc/bus/pci/X/Y mmap()s. */

/* If the user uses a host-bridge as the PCI device, he may use
 * this to perform a raw mmap() of the I/O or MEM space behind
 * that controller.
 *
 * This can be useful for execution of x86 PCI bios initialization code
 * on a PCI card, like the xfree86 int10 stuff does.
 */
static int __pci_mmap_make_offset_bus(struct pci_dev *pdev, struct vm_area_struct *vma,
				      enum pci_mmap_state mmap_state)
{
	struct pci_pbm_info *pbm = pdev->dev.archdata.host_controller;
	unsigned long space_size, user_offset, user_size;

	if (mmap_state == pci_mmap_io) {
		space_size = resource_size(&pbm->io_space);
	} else {
		space_size = resource_size(&pbm->mem_space);
	}

	/* Make sure the request is in range. */
	user_offset = vma->vm_pgoff << PAGE_SHIFT;
	user_size = vma->vm_end - vma->vm_start;

	if (user_offset >= space_size ||
	    (user_offset + user_size) > space_size)
		return -EINVAL;

	if (mmap_state == pci_mmap_io) {
		vma->vm_pgoff = (pbm->io_space.start +
				 user_offset) >> PAGE_SHIFT;
	} else {
		vma->vm_pgoff = (pbm->mem_space.start +
				 user_offset) >> PAGE_SHIFT;
	}