			 pr, (pr && pr->name) ? pr->name : "nil");

		if (pr && !(pr->flags & IORESOURCE_UNSET)) {
			if (request_resource(pr, res) == 0)
				continue;
			/*
			 * Must be a conflict with an existing entry.
			 */
			if (reparent_resources(pr, res) == 0)
				continue;
		}
		pr_warning("PCI: Cannot allocate resource region "
			   "%d of PCI bridge %d, will remap\n", i, bus->number);
	clear_resource:
				 (unsigned long long)r->end,
				 (unsigned int)r->flags);

			pci_claim_resource(dev, i);
		}
	}

	list_for_each_entry(child_bus, &bus->children, node)