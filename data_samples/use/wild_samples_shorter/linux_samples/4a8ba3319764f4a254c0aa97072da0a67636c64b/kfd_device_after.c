		goto kfd_topology_add_device_error;
	}

	if (!device_iommu_pasid_init(kfd)) {
		dev_err(kfd_device,
			"Error initializing iommuv2 for device (%x:%x)\n",
			kfd->pdev->vendor, kfd->pdev->device);
device_queue_manager_error:
	amd_iommu_free_device(kfd->pdev);
device_iommu_pasid_error:
	kfd_topology_remove_device(kfd);
kfd_topology_add_device_error:
	kfd2kgd->fini_sa_manager(kfd->kgd);
	dev_err(kfd_device,
	if (kfd->init_complete) {
		device_queue_manager_uninit(kfd->dqm);
		amd_iommu_free_device(kfd->pdev);
		kfd_topology_remove_device(kfd);
	}

	kfree(kfd);
/* This is called directly from KGD at ISR. */
void kgd2kfd_interrupt(struct kfd_dev *kfd, const void *ih_ring_entry)
{
	/* Process interrupts / schedule work as necessary */
}