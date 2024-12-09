	struct pci_dev *pdev = vdev->pdev;
	unsigned int flag = msix ? PCI_IRQ_MSIX : PCI_IRQ_MSI;
	int ret;
	u16 cmd;

	if (!is_irq_none(vdev))
		return -EINVAL;

		return -ENOMEM;

	/* return the number of supported vectors if we can't get all: */
	cmd = vfio_pci_memory_lock_and_enable(vdev);
	ret = pci_alloc_irq_vectors(pdev, 1, nvec, flag);
	if (ret < nvec) {
		if (ret > 0)
			pci_free_irq_vectors(pdev);
		vfio_pci_memory_unlock_and_restore(vdev, cmd);
		kfree(vdev->ctx);
		return ret;
	}
	vfio_pci_memory_unlock_and_restore(vdev, cmd);

	vdev->num_ctx = nvec;
	vdev->irq_type = msix ? VFIO_PCI_MSIX_IRQ_INDEX :
				VFIO_PCI_MSI_IRQ_INDEX;
	struct pci_dev *pdev = vdev->pdev;
	struct eventfd_ctx *trigger;
	int irq, ret;
	u16 cmd;

	if (vector < 0 || vector >= vdev->num_ctx)
		return -EINVAL;


	if (vdev->ctx[vector].trigger) {
		irq_bypass_unregister_producer(&vdev->ctx[vector].producer);

		cmd = vfio_pci_memory_lock_and_enable(vdev);
		free_irq(irq, vdev->ctx[vector].trigger);
		vfio_pci_memory_unlock_and_restore(vdev, cmd);

		kfree(vdev->ctx[vector].name);
		eventfd_ctx_put(vdev->ctx[vector].trigger);
		vdev->ctx[vector].trigger = NULL;
	}
	 * such a reset it would be unsuccessful. To avoid this, restore the
	 * cached value of the message prior to enabling.
	 */
	cmd = vfio_pci_memory_lock_and_enable(vdev);
	if (msix) {
		struct msi_msg msg;

		get_cached_msi_msg(irq, &msg);

	ret = request_irq(irq, vfio_msihandler, 0,
			  vdev->ctx[vector].name, trigger);
	vfio_pci_memory_unlock_and_restore(vdev, cmd);
	if (ret) {
		kfree(vdev->ctx[vector].name);
		eventfd_ctx_put(trigger);
		return ret;
{
	struct pci_dev *pdev = vdev->pdev;
	int i;
	u16 cmd;

	for (i = 0; i < vdev->num_ctx; i++) {
		vfio_virqfd_disable(&vdev->ctx[i].unmask);
		vfio_virqfd_disable(&vdev->ctx[i].mask);

	vfio_msi_set_block(vdev, 0, vdev->num_ctx, NULL, msix);

	cmd = vfio_pci_memory_lock_and_enable(vdev);
	pci_free_irq_vectors(pdev);
	vfio_pci_memory_unlock_and_restore(vdev, cmd);

	/*
	 * Both disable paths above use pci_intx_for_msi() to clear DisINTx
	 * via their shutdown paths.  Restore for NoINTx devices.