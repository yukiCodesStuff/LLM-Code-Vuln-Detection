
static void do_detach(struct iommu_dev_data *dev_data)
{
	struct amd_iommu *iommu;
	u16 alias;

	iommu = amd_iommu_rlookup_table[dev_data->devid];
	alias = dev_data->alias;

	/* decrease reference counters */
	dev_data->domain->dev_iommu[iommu->index] -= 1;
	dev_data->domain->dev_cnt                 -= 1;

	/* Update data structures */
	dev_data->domain = NULL;
	list_del(&dev_data->list);
	clear_dte_entry(dev_data->devid);

	/* Flush the DTE entry */
	device_flush_dte(dev_data);
}

/*
 * If a device is not yet associated with a domain, this function makes the
			bus_addr  = address + s->dma_address + (j << PAGE_SHIFT);
			iommu_unmap_page(domain, bus_addr, PAGE_SIZE);

			if (--mapped_pages)
				goto out_free_iova;
		}
	}

out_free_iova:
	free_iova_fast(&dma_dom->iovad, address, npages);

out_err:
	return 0;
}