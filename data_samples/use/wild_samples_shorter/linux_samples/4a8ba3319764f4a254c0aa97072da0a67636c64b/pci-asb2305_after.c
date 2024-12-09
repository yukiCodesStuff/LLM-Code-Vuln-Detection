				if (!r->flags)
					continue;
				if (!r->start ||
				    pci_claim_bridge_resource(dev, idx) < 0) {
					printk(KERN_ERR "PCI:"
					       " Cannot allocate resource"
					       " region %d of bridge %s\n",
					       idx, pci_name(dev));