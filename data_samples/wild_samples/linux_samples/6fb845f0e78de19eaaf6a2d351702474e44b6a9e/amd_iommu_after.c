{
	struct amd_iommu *iommu;
	u16 alias;
	bool ats;

	iommu = amd_iommu_rlookup_table[dev_data->devid];
	alias = dev_data->alias;
	ats   = dev_data->ats.enabled;

	/* Update data structures */
	dev_data->domain = domain;
	list_add(&dev_data->list, &domain->dev_list);

	/* Do reference counting */
	domain->dev_iommu[iommu->index] += 1;
	domain->dev_cnt                 += 1;

	/* Update device table */
	set_dte_entry(dev_data->devid, domain, ats, dev_data->iommu_v2);
	if (alias != dev_data->devid)
		set_dte_entry(alias, domain, ats, dev_data->iommu_v2);

	device_flush_dte(dev_data);
}

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
	if (alias != dev_data->devid)
		clear_dte_entry(alias);

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
	if (alias != dev_data->devid)
		clear_dte_entry(alias);

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
 * device visible in the domain
 */
static int __attach_device(struct iommu_dev_data *dev_data,
			   struct protection_domain *domain)
{
	int ret;

	/* lock domain */
	spin_lock(&domain->lock);

	ret = -EBUSY;
	if (dev_data->domain != NULL)
		goto out_unlock;

	/* Attach alias group root */
	do_attach(dev_data, domain);

	ret = 0;

out_unlock:

	/* ready */
	spin_unlock(&domain->lock);

	return ret;
}


static void pdev_iommuv2_disable(struct pci_dev *pdev)
{
	pci_disable_ats(pdev);
	pci_disable_pri(pdev);
	pci_disable_pasid(pdev);
}

/* FIXME: Change generic reset-function to do the same */
static int pri_reset_while_enabled(struct pci_dev *pdev)
{
	u16 control;
	int pos;

	pos = pci_find_ext_capability(pdev, PCI_EXT_CAP_ID_PRI);
	if (!pos)
		return -EINVAL;

	pci_read_config_word(pdev, pos + PCI_PRI_CTRL, &control);
	control |= PCI_PRI_CTRL_RESET;
	pci_write_config_word(pdev, pos + PCI_PRI_CTRL, control);

	return 0;
}

static int pdev_iommuv2_enable(struct pci_dev *pdev)
{
	bool reset_enable;
	int reqs, ret;

	/* FIXME: Hardcode number of outstanding requests for now */
	reqs = 32;
	if (pdev_pri_erratum(pdev, AMD_PRI_DEV_ERRATUM_LIMIT_REQ_ONE))
		reqs = 1;
	reset_enable = pdev_pri_erratum(pdev, AMD_PRI_DEV_ERRATUM_ENABLE_RESET);

	/* Only allow access to user-accessible pages */
	ret = pci_enable_pasid(pdev, 0);
	if (ret)
		goto out_err;

	/* First reset the PRI state of the device */
	ret = pci_reset_pri(pdev);
	if (ret)
		goto out_err;

	/* Enable PRI */
	ret = pci_enable_pri(pdev, reqs);
	if (ret)
		goto out_err;

	if (reset_enable) {
		ret = pri_reset_while_enabled(pdev);
		if (ret)
			goto out_err;
	}
		for (j = 0; j < pages; ++j) {
			unsigned long bus_addr;

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

/*
 * The exported map_sg function for dma_ops (handles scatter-gather
 * lists).
 */
static void unmap_sg(struct device *dev, struct scatterlist *sglist,
		     int nelems, enum dma_data_direction dir,
		     unsigned long attrs)
{
	struct protection_domain *domain;
	struct dma_ops_domain *dma_dom;
	unsigned long startaddr;
	int npages = 2;

	domain = get_domain(dev);
	if (IS_ERR(domain))
		return;

	startaddr = sg_dma_address(sglist) & PAGE_MASK;
	dma_dom   = to_dma_ops_domain(domain);
	npages    = sg_num_pages(dev, sglist, nelems);

	__unmap_single(dma_dom, startaddr, npages << PAGE_SHIFT, dir);
}

/*
 * The exported alloc_coherent function for dma_ops.
 */
static void *alloc_coherent(struct device *dev, size_t size,
			    dma_addr_t *dma_addr, gfp_t flag,
			    unsigned long attrs)
{
	u64 dma_mask = dev->coherent_dma_mask;
	struct protection_domain *domain;
	struct dma_ops_domain *dma_dom;
	struct page *page;

	domain = get_domain(dev);
	if (PTR_ERR(domain) == -EINVAL) {
		page = alloc_pages(flag, get_order(size));
		*dma_addr = page_to_phys(page);
		return page_address(page);
	} else if (IS_ERR(domain))
		return NULL;

	dma_dom   = to_dma_ops_domain(domain);
	size	  = PAGE_ALIGN(size);
	dma_mask  = dev->coherent_dma_mask;
	flag     &= ~(__GFP_DMA | __GFP_HIGHMEM | __GFP_DMA32);
	flag     |= __GFP_ZERO;

	page = alloc_pages(flag | __GFP_NOWARN,  get_order(size));
	if (!page) {
		if (!gfpflags_allow_blocking(flag))
			return NULL;

		page = dma_alloc_from_contiguous(dev, size >> PAGE_SHIFT,
					get_order(size), flag & __GFP_NOWARN);
		if (!page)
			return NULL;
	}

	if (!dma_mask)
		dma_mask = *dev->dma_mask;

	*dma_addr = __map_single(dev, dma_dom, page_to_phys(page),
				 size, DMA_BIDIRECTIONAL, dma_mask);

	if (*dma_addr == DMA_MAPPING_ERROR)
		goto out_free;

	return page_address(page);

out_free:

	if (!dma_release_from_contiguous(dev, page, size >> PAGE_SHIFT))
		__free_pages(page, get_order(size));

	return NULL;
}

/*
 * The exported free_coherent function for dma_ops.
 */
static void free_coherent(struct device *dev, size_t size,
			  void *virt_addr, dma_addr_t dma_addr,
			  unsigned long attrs)
{
	struct protection_domain *domain;
	struct dma_ops_domain *dma_dom;
	struct page *page;

	page = virt_to_page(virt_addr);
	size = PAGE_ALIGN(size);

	domain = get_domain(dev);
	if (IS_ERR(domain))
		goto free_mem;

	dma_dom = to_dma_ops_domain(domain);

	__unmap_single(dma_dom, dma_addr, size, DMA_BIDIRECTIONAL);

free_mem:
	if (!dma_release_from_contiguous(dev, page, size >> PAGE_SHIFT))
		__free_pages(page, get_order(size));
}

/*
 * This function is called by the DMA layer to find out if we can handle a
 * particular device. It is part of the dma_ops.
 */
static int amd_iommu_dma_supported(struct device *dev, u64 mask)
{
	if (!dma_direct_supported(dev, mask))
		return 0;
	return check_device(dev);
}

static const struct dma_map_ops amd_iommu_dma_ops = {
	.alloc		= alloc_coherent,
	.free		= free_coherent,
	.map_page	= map_page,
	.unmap_page	= unmap_page,
	.map_sg		= map_sg,
	.unmap_sg	= unmap_sg,
	.dma_supported	= amd_iommu_dma_supported,
};

static int init_reserved_iova_ranges(void)
{
	struct pci_dev *pdev = NULL;
	struct iova *val;

	init_iova_domain(&reserved_iova_ranges, PAGE_SIZE, IOVA_START_PFN);

	lockdep_set_class(&reserved_iova_ranges.iova_rbtree_lock,
			  &reserved_rbtree_key);

	/* MSI memory range */
	val = reserve_iova(&reserved_iova_ranges,
			   IOVA_PFN(MSI_RANGE_START), IOVA_PFN(MSI_RANGE_END));
	if (!val) {
		pr_err("Reserving MSI range failed\n");
		return -ENOMEM;
	}