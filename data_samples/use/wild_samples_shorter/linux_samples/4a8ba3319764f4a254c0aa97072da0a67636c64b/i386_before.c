			continue;
		if (r->parent)	/* Already allocated */
			continue;
		if (!r->start || pci_claim_resource(dev, idx) < 0) {
			/*
			 * Something is wrong with the region.
			 * Invalidate the resource to prevent
			 * child resource allocations in this