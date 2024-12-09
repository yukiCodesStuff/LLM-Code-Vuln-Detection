
static void do_detach(struct iommu_dev_data *dev_data)
{
	struct protection_domain *domain = dev_data->domain;
	struct amd_iommu *iommu;
	u16 alias;

	iommu = amd_iommu_rlookup_table[dev_data->devid];
	alias = dev_data->alias;

	/* Update data structures */
	dev_data->domain = NULL;
	list_del(&dev_data->list);
	clear_dte_entry(dev_data->devid);

	/* Flush the DTE entry */
	device_flush_dte(dev_data);

	/* Flush IOTLB */
	domain_flush_tlb_pde(domain);

	/* Wait for the flushes to finish */
	domain_flush_complete(domain);

	/* decrease reference counters - needs to happen after the flushes */
	domain->dev_iommu[iommu->index] -= 1;
	domain->dev_cnt                 -= 1;
}

/*
 * If a device is not yet associated with a domain, this function makes the
			bus_addr  = address + s->dma_address + (j << PAGE_SHIFT);
			iommu_unmap_page(domain, bus_addr, PAGE_SIZE);

			if (--mapped_pages == 0)
				goto out_free_iova;
		}
	}

out_free_iova:
	free_iova_fast(&dma_dom->iovad, address >> PAGE_SHIFT, npages);

out_err:
	return 0;
}