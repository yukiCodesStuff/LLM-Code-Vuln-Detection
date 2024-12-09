			 bus->self ? pci_name(bus->self) : "PHB",
			 bus->number, i,
			 (unsigned long long)res->start,
			 (unsigned long long)res->end,
			 (unsigned int)res->flags,
			 pr, (pr && pr->name) ? pr->name : "nil");

		if (pr && !(pr->flags & IORESOURCE_UNSET)) {
			if (request_resource(pr, res) == 0)
				continue;
			/*
			 * Must be a conflict with an existing entry.
			 * Move that entry (or entries) under the
			 * bridge resource and try again.
			 */
			if (reparent_resources(pr, res) == 0)
				continue;
		}
		if (pr && !(pr->flags & IORESOURCE_UNSET)) {
			if (request_resource(pr, res) == 0)
				continue;
			/*
			 * Must be a conflict with an existing entry.
			 * Move that entry (or entries) under the
			 * bridge resource and try again.
			 */
			if (reparent_resources(pr, res) == 0)
				continue;
		}
		pr_warning("PCI: Cannot allocate resource region "
			   "%d of PCI bridge %d, will remap\n", i, bus->number);
	clear_resource:
		/* The resource might be figured out when doing
		 * reassignment based on the resources required
		 * by the downstream PCI devices. Here we set
		 * the size of the resource to be 0 in order to
		 * save more space.
		 */
		res->start = 0;
		res->end = -1;
		res->flags = 0;
	}

	list_for_each_entry(b, &bus->children, node)
		pcibios_allocate_bus_resources(b);
}

static inline void alloc_resource(struct pci_dev *dev, int idx)
{
	struct resource *pr, *r = &dev->resource[idx];

	pr_debug("PCI: Allocating %s: Resource %d: %016llx..%016llx [%x]\n",
		 pci_name(dev), idx,
		 (unsigned long long)r->start,
		 (unsigned long long)r->end,
		 (unsigned int)r->flags);

	pr = pci_find_parent_resource(dev, r);
	if (!pr || (pr->flags & IORESOURCE_UNSET) ||
	    request_resource(pr, r) < 0) {
		printk(KERN_WARNING "PCI: Cannot allocate resource region %d"
		       " of device %s, will remap\n", idx, pci_name(dev));
		if (pr)
			pr_debug("PCI:  parent is %p: %016llx-%016llx [%x]\n",
				 pr,
				 (unsigned long long)pr->start,
				 (unsigned long long)pr->end,
				 (unsigned int)pr->flags);
		/* We'll assign a new address later */
		r->flags |= IORESOURCE_UNSET;
		r->end -= r->start;
		r->start = 0;
	}
}

static void __init pcibios_allocate_resources(int pass)
{
	struct pci_dev *dev = NULL;
	int idx, disabled;
	u16 command;
	struct resource *r;

	for_each_pci_dev(dev) {
		pci_read_config_word(dev, PCI_COMMAND, &command);
		for (idx = 0; idx <= PCI_ROM_RESOURCE; idx++) {
			r = &dev->resource[idx];
			if (r->parent)		/* Already allocated */
				continue;
			if (!r->flags || (r->flags & IORESOURCE_UNSET))
				continue;	/* Not assigned at all */
			/* We only allocate ROMs on pass 1 just in case they
			 * have been screwed up by firmware
			 */
			if (idx == PCI_ROM_RESOURCE )
				disabled = 1;
			if (r->flags & IORESOURCE_IO)
				disabled = !(command & PCI_COMMAND_IO);
			else
				disabled = !(command & PCI_COMMAND_MEMORY);
			if (pass == disabled)
				alloc_resource(dev, idx);
		}
		if (pass)
			continue;
		r = &dev->resource[PCI_ROM_RESOURCE];
		if (r->flags) {
			/* Turn the ROM off, leave the resource region,
			 * but keep it unregistered.
			 */
			u32 reg;
			pci_read_config_dword(dev, dev->rom_base_reg, &reg);
			if (reg & PCI_ROM_ADDRESS_ENABLE) {
				pr_debug("PCI: Switching off ROM of %s\n",
					 pci_name(dev));
				r->flags &= ~IORESOURCE_ROM_ENABLE;
				pci_write_config_dword(dev, dev->rom_base_reg,
						       reg & ~PCI_ROM_ADDRESS_ENABLE);
			}
		for (i = 0; i < PCI_NUM_RESOURCES; i++) {
			struct resource *r = &dev->resource[i];

			if (r->parent || !r->start || !r->flags)
				continue;

			pr_debug("PCI: Claiming %s: "
				 "Resource %d: %016llx..%016llx [%x]\n",
				 pci_name(dev), i,
				 (unsigned long long)r->start,
				 (unsigned long long)r->end,
				 (unsigned int)r->flags);

			pci_claim_resource(dev, i);
		}
	}

	list_for_each_entry(child_bus, &bus->children, node)
		pcibios_claim_one_bus(child_bus);
}


/* pcibios_finish_adding_to_bus
 *
 * This is to be called by the hotplug code after devices have been
 * added to a bus, this include calling it for a PHB that is just
 * being added
 */
void pcibios_finish_adding_to_bus(struct pci_bus *bus)
{
	pr_debug("PCI: Finishing adding to hotplug bus %04x:%02x\n",
		 pci_domain_nr(bus), bus->number);

	/* Allocate bus and devices resources */
	pcibios_allocate_bus_resources(bus);
	pcibios_claim_one_bus(bus);
	if (!pci_has_flag(PCI_PROBE_ONLY))
		pci_assign_unassigned_bus_resources(bus);

	/* Fixup EEH */
	eeh_add_device_tree_late(bus);

	/* Add new devices to global lists.  Register in proc, sysfs. */
	pci_bus_add_devices(bus);

	/* sysfs files should only be added after devices are added */
	eeh_add_sysfs_files(bus);
}
EXPORT_SYMBOL_GPL(pcibios_finish_adding_to_bus);

int pcibios_enable_device(struct pci_dev *dev, int mask)
{
	if (ppc_md.pcibios_enable_device_hook)
		if (ppc_md.pcibios_enable_device_hook(dev))
			return -EINVAL;

	return pci_enable_resources(dev, mask);
}

resource_size_t pcibios_io_space_offset(struct pci_controller *hose)
{
	return (unsigned long) hose->io_base_virt - _IO_BASE;
}

static void pcibios_setup_phb_resources(struct pci_controller *hose,
					struct list_head *resources)
{
	struct resource *res;
	resource_size_t offset;
	int i;

	/* Hookup PHB IO resource */
	res = &hose->io_resource;

	if (!res->flags) {
		pr_info("PCI: I/O resource not set for host"
		       " bridge %s (domain %d)\n",
		       hose->dn->full_name, hose->global_number);
	} else {
		offset = pcibios_io_space_offset(hose);

		pr_debug("PCI: PHB IO resource    = %08llx-%08llx [%lx] off 0x%08llx\n",
			 (unsigned long long)res->start,
			 (unsigned long long)res->end,
			 (unsigned long)res->flags,
			 (unsigned long long)offset);
		pci_add_resource_offset(resources, res, offset);
	}

	/* Hookup PHB Memory resources */
	for (i = 0; i < 3; ++i) {
		res = &hose->mem_resources[i];
		if (!res->flags) {
			if (i == 0)
				printk(KERN_ERR "PCI: Memory resource 0 not set for "
				       "host bridge %s (domain %d)\n",
				       hose->dn->full_name, hose->global_number);
			continue;
		}