	for (idx = PCI_BRIDGE_RESOURCES; idx < PCI_NUM_RESOURCES; idx++) {
		r = &dev->resource[idx];
		if (!r->flags)
			continue;
		if (r->parent)	/* Already allocated */
			continue;
		if (!r->start || pci_claim_resource(dev, idx) < 0) {
			/*
			 * Something is wrong with the region.
			 * Invalidate the resource to prevent
			 * child resource allocations in this
			 * range.
			 */
			r->start = r->end = 0;
			r->flags = 0;
		}