			 pr, (pr && pr->name) ? pr->name : "nil");

		if (pr && !(pr->flags & IORESOURCE_UNSET)) {
			struct pci_dev *dev = bus->self;

			if (request_resource(pr, res) == 0)
				continue;
			/*
			 * Must be a conflict with an existing entry.
			 */
			if (reparent_resources(pr, res) == 0)
				continue;

			if (dev && i < PCI_BRIDGE_RESOURCE_NUM &&
			    pci_claim_bridge_resource(dev,
						i + PCI_BRIDGE_RESOURCES) == 0)
				continue;
		}
		pr_warning("PCI: Cannot allocate resource region "
			   "%d of PCI bridge %d, will remap\n", i, bus->number);
	clear_resource:
				 (unsigned long long)r->end,
				 (unsigned int)r->flags);

			if (pci_claim_resource(dev, i) == 0)
				continue;

			pci_claim_bridge_resource(dev, i);
		}
	}

	list_for_each_entry(child_bus, &bus->children, node)