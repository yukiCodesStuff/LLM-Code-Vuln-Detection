	if (!found) {
		pr_warn("tce_vfio: detaching unattached group #%u\n",
				iommu_group_id(iommu_group));
		goto unlock_exit;
	}

	list_del(&tcegrp->next);
	kfree(tcegrp);

	table_group = iommu_group_get_iommudata(iommu_group);
	BUG_ON(!table_group);

	if (!table_group->ops || !table_group->ops->release_ownership)
		tce_iommu_release_ownership(container, table_group);
	else
		tce_iommu_release_ownership_ddw(container, table_group);

unlock_exit:
	mutex_unlock(&container->lock);
}

static const struct vfio_iommu_driver_ops tce_iommu_driver_ops = {
	.name		= "iommu-vfio-powerpc",
	.owner		= THIS_MODULE,
	.open		= tce_iommu_open,
	.release	= tce_iommu_release,
	.ioctl		= tce_iommu_ioctl,
	.attach_group	= tce_iommu_attach_group,
	.detach_group	= tce_iommu_detach_group,
};

static int __init tce_iommu_init(void)
{
	return vfio_register_iommu_driver(&tce_iommu_driver_ops);
}