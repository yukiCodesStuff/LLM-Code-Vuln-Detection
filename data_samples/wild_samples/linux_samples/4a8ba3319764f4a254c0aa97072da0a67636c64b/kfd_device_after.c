	if (kfd_topology_add_device(kfd) != 0) {
		dev_err(kfd_device,
			"Error adding device (%x:%x) to topology\n",
			kfd->pdev->vendor, kfd->pdev->device);
		goto kfd_topology_add_device_error;
	}

	if (!device_iommu_pasid_init(kfd)) {
		dev_err(kfd_device,
			"Error initializing iommuv2 for device (%x:%x)\n",
			kfd->pdev->vendor, kfd->pdev->device);
		goto device_iommu_pasid_error;
	}
	amd_iommu_set_invalidate_ctx_cb(kfd->pdev,
						iommu_pasid_shutdown_callback);

	kfd->dqm = device_queue_manager_init(kfd);
	if (!kfd->dqm) {
		dev_err(kfd_device,
			"Error initializing queue manager for device (%x:%x)\n",
			kfd->pdev->vendor, kfd->pdev->device);
		goto device_queue_manager_error;
	}

	if (kfd->dqm->start(kfd->dqm) != 0) {
		dev_err(kfd_device,
			"Error starting queuen manager for device (%x:%x)\n",
			kfd->pdev->vendor, kfd->pdev->device);
		goto dqm_start_error;
	}

	kfd->init_complete = true;
	dev_info(kfd_device, "added device (%x:%x)\n", kfd->pdev->vendor,
		 kfd->pdev->device);

	pr_debug("kfd: Starting kfd with the following scheduling policy %d\n",
		sched_policy);

	goto out;

dqm_start_error:
	device_queue_manager_uninit(kfd->dqm);
device_queue_manager_error:
	amd_iommu_free_device(kfd->pdev);
device_iommu_pasid_error:
	kfd_topology_remove_device(kfd);
kfd_topology_add_device_error:
	kfd2kgd->fini_sa_manager(kfd->kgd);
	dev_err(kfd_device,
		"device (%x:%x) NOT added due to errors\n",
		kfd->pdev->vendor, kfd->pdev->device);
out:
	return kfd->init_complete;
}

void kgd2kfd_device_exit(struct kfd_dev *kfd)
{
	if (kfd->init_complete) {
		device_queue_manager_uninit(kfd->dqm);
		amd_iommu_free_device(kfd->pdev);
		kfd_topology_remove_device(kfd);
	}

	kfree(kfd);
}

void kgd2kfd_suspend(struct kfd_dev *kfd)
{
	BUG_ON(kfd == NULL);

	if (kfd->init_complete) {
		kfd->dqm->stop(kfd->dqm);
		amd_iommu_set_invalidate_ctx_cb(kfd->pdev, NULL);
		amd_iommu_free_device(kfd->pdev);
	}
}

int kgd2kfd_resume(struct kfd_dev *kfd)
{
	unsigned int pasid_limit;
	int err;

	BUG_ON(kfd == NULL);

	pasid_limit = kfd_get_pasid_limit();

	if (kfd->init_complete) {
		err = amd_iommu_init_device(kfd->pdev, pasid_limit);
		if (err < 0)
			return -ENXIO;
		amd_iommu_set_invalidate_ctx_cb(kfd->pdev,
						iommu_pasid_shutdown_callback);
		kfd->dqm->start(kfd->dqm);
	}

	return 0;
}

/* This is called directly from KGD at ISR. */
void kgd2kfd_interrupt(struct kfd_dev *kfd, const void *ih_ring_entry)
{
	/* Process interrupts / schedule work as necessary */
}
	if (kfd->dqm->start(kfd->dqm) != 0) {
		dev_err(kfd_device,
			"Error starting queuen manager for device (%x:%x)\n",
			kfd->pdev->vendor, kfd->pdev->device);
		goto dqm_start_error;
	}

	kfd->init_complete = true;
	dev_info(kfd_device, "added device (%x:%x)\n", kfd->pdev->vendor,
		 kfd->pdev->device);

	pr_debug("kfd: Starting kfd with the following scheduling policy %d\n",
		sched_policy);

	goto out;

dqm_start_error:
	device_queue_manager_uninit(kfd->dqm);
device_queue_manager_error:
	amd_iommu_free_device(kfd->pdev);
device_iommu_pasid_error:
	kfd_topology_remove_device(kfd);
kfd_topology_add_device_error:
	kfd2kgd->fini_sa_manager(kfd->kgd);
	dev_err(kfd_device,
		"device (%x:%x) NOT added due to errors\n",
		kfd->pdev->vendor, kfd->pdev->device);
out:
	return kfd->init_complete;
}

void kgd2kfd_device_exit(struct kfd_dev *kfd)
{
	if (kfd->init_complete) {
		device_queue_manager_uninit(kfd->dqm);
		amd_iommu_free_device(kfd->pdev);
		kfd_topology_remove_device(kfd);
	}
	if (kfd->init_complete) {
		device_queue_manager_uninit(kfd->dqm);
		amd_iommu_free_device(kfd->pdev);
		kfd_topology_remove_device(kfd);
	}

	kfree(kfd);
}

void kgd2kfd_suspend(struct kfd_dev *kfd)
{
	BUG_ON(kfd == NULL);

	if (kfd->init_complete) {
		kfd->dqm->stop(kfd->dqm);
		amd_iommu_set_invalidate_ctx_cb(kfd->pdev, NULL);
		amd_iommu_free_device(kfd->pdev);
	}
}

int kgd2kfd_resume(struct kfd_dev *kfd)
{
	unsigned int pasid_limit;
	int err;

	BUG_ON(kfd == NULL);

	pasid_limit = kfd_get_pasid_limit();

	if (kfd->init_complete) {
		err = amd_iommu_init_device(kfd->pdev, pasid_limit);
		if (err < 0)
			return -ENXIO;
		amd_iommu_set_invalidate_ctx_cb(kfd->pdev,
						iommu_pasid_shutdown_callback);
		kfd->dqm->start(kfd->dqm);
	}

	return 0;
}

/* This is called directly from KGD at ISR. */
void kgd2kfd_interrupt(struct kfd_dev *kfd, const void *ih_ring_entry)
{
	/* Process interrupts / schedule work as necessary */
}
	if (kfd->init_complete) {
		err = amd_iommu_init_device(kfd->pdev, pasid_limit);
		if (err < 0)
			return -ENXIO;
		amd_iommu_set_invalidate_ctx_cb(kfd->pdev,
						iommu_pasid_shutdown_callback);
		kfd->dqm->start(kfd->dqm);
	}

	return 0;
}

/* This is called directly from KGD at ISR. */
void kgd2kfd_interrupt(struct kfd_dev *kfd, const void *ih_ring_entry)
{
	/* Process interrupts / schedule work as necessary */
}