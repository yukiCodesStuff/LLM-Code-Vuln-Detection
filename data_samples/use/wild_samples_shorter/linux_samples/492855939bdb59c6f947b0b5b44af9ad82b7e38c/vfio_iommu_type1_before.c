MODULE_PARM_DESC(disable_hugepages,
		 "Disable VFIO IOMMU support for IOMMU hugepages.");

struct vfio_iommu {
	struct list_head	domain_list;
	struct vfio_domain	*external_domain; /* domain for external user */
	struct mutex		lock;
	struct rb_root		dma_list;
	struct blocking_notifier_head notifier;
	bool			v2;
	bool			nesting;
};

	vfio_unlink_dma(iommu, dma);
	put_task_struct(dma->task);
	kfree(dma);
}

static unsigned long vfio_pgsize_bitmap(struct vfio_iommu *iommu)
{
		goto out_unlock;
	}

	dma = kzalloc(sizeof(*dma), GFP_KERNEL);
	if (!dma) {
		ret = -ENOMEM;
		goto out_unlock;
	}

	dma->iova = iova;
	dma->vaddr = vaddr;
	dma->prot = prot;


	INIT_LIST_HEAD(&iommu->domain_list);
	iommu->dma_list = RB_ROOT;
	mutex_init(&iommu->lock);
	BLOCKING_INIT_NOTIFIER_HEAD(&iommu->notifier);

	return iommu;