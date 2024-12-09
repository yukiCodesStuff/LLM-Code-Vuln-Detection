				       (unsigned long long)r->end,
				       (unsigned int)r->flags);

			if (pci_claim_resource(dev, i) == 0)
				continue;

			pci_claim_bridge_resource(dev, i);
		}
	}

	list_for_each_entry(child_bus, &bus->children, node)