{
	struct pci_dev *pdev = vdev->pdev;
	unsigned int flag = msix ? PCI_IRQ_MSIX : PCI_IRQ_MSI;
	int ret;
	u16 cmd;

	if (!is_irq_none(vdev))
		return -EINVAL;

	vdev->ctx = kcalloc(nvec, sizeof(struct vfio_pci_irq_ctx), GFP_KERNEL);
	if (!vdev->ctx)
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
{
	struct pci_dev *pdev = vdev->pdev;
	unsigned int flag = msix ? PCI_IRQ_MSIX : PCI_IRQ_MSI;
	int ret;
	u16 cmd;

	if (!is_irq_none(vdev))
		return -EINVAL;

	vdev->ctx = kcalloc(nvec, sizeof(struct vfio_pci_irq_ctx), GFP_KERNEL);
	if (!vdev->ctx)
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
{
	struct pci_dev *pdev = vdev->pdev;
	struct eventfd_ctx *trigger;
	int irq, ret;
	u16 cmd;

	if (vector < 0 || vector >= vdev->num_ctx)
		return -EINVAL;

	irq = pci_irq_vector(pdev, vector);

	if (vdev->ctx[vector].trigger) {
		irq_bypass_unregister_producer(&vdev->ctx[vector].producer);

		cmd = vfio_pci_memory_lock_and_enable(vdev);
		free_irq(irq, vdev->ctx[vector].trigger);
		vfio_pci_memory_unlock_and_restore(vdev, cmd);

		kfree(vdev->ctx[vector].name);
		eventfd_ctx_put(vdev->ctx[vector].trigger);
		vdev->ctx[vector].trigger = NULL;
	}
	if (vdev->ctx[vector].trigger) {
		irq_bypass_unregister_producer(&vdev->ctx[vector].producer);

		cmd = vfio_pci_memory_lock_and_enable(vdev);
		free_irq(irq, vdev->ctx[vector].trigger);
		vfio_pci_memory_unlock_and_restore(vdev, cmd);

		kfree(vdev->ctx[vector].name);
		eventfd_ctx_put(vdev->ctx[vector].trigger);
		vdev->ctx[vector].trigger = NULL;
	}
	if (IS_ERR(trigger)) {
		kfree(vdev->ctx[vector].name);
		return PTR_ERR(trigger);
	}

	/*
	 * The MSIx vector table resides in device memory which may be cleared
	 * via backdoor resets. We don't allow direct access to the vector
	 * table so even if a userspace driver attempts to save/restore around
	 * such a reset it would be unsuccessful. To avoid this, restore the
	 * cached value of the message prior to enabling.
	 */
	cmd = vfio_pci_memory_lock_and_enable(vdev);
	if (msix) {
		struct msi_msg msg;

		get_cached_msi_msg(irq, &msg);
		pci_write_msi_msg(irq, &msg);
	}
	if (msix) {
		struct msi_msg msg;

		get_cached_msi_msg(irq, &msg);
		pci_write_msi_msg(irq, &msg);
	}

	ret = request_irq(irq, vfio_msihandler, 0,
			  vdev->ctx[vector].name, trigger);
	vfio_pci_memory_unlock_and_restore(vdev, cmd);
	if (ret) {
		kfree(vdev->ctx[vector].name);
		eventfd_ctx_put(trigger);
		return ret;
	}
{
	struct pci_dev *pdev = vdev->pdev;
	int i;
	u16 cmd;

	for (i = 0; i < vdev->num_ctx; i++) {
		vfio_virqfd_disable(&vdev->ctx[i].unmask);
		vfio_virqfd_disable(&vdev->ctx[i].mask);
	}
	for (i = 0; i < vdev->num_ctx; i++) {
		vfio_virqfd_disable(&vdev->ctx[i].unmask);
		vfio_virqfd_disable(&vdev->ctx[i].mask);
	}

	vfio_msi_set_block(vdev, 0, vdev->num_ctx, NULL, msix);

	cmd = vfio_pci_memory_lock_and_enable(vdev);
	pci_free_irq_vectors(pdev);
	vfio_pci_memory_unlock_and_restore(vdev, cmd);

	/*
	 * Both disable paths above use pci_intx_for_msi() to clear DisINTx
	 * via their shutdown paths.  Restore for NoINTx devices.
	 */
	if (vdev->nointx)
		pci_intx(pdev, 0);

	vdev->irq_type = VFIO_PCI_NUM_IRQS;
	vdev->num_ctx = 0;
	kfree(vdev->ctx);
}

/*
 * IOCTL support
 */
static int vfio_pci_set_intx_unmask(struct vfio_pci_device *vdev,
				    unsigned index, unsigned start,
				    unsigned count, uint32_t flags, void *data)
{
	if (!is_intx(vdev) || start != 0 || count != 1)
		return -EINVAL;

	if (flags & VFIO_IRQ_SET_DATA_NONE) {
		vfio_pci_intx_unmask(vdev);
	} else if (flags & VFIO_IRQ_SET_DATA_BOOL) {
		uint8_t unmask = *(uint8_t *)data;
		if (unmask)
			vfio_pci_intx_unmask(vdev);
	} else if (flags & VFIO_IRQ_SET_DATA_EVENTFD) {
		int32_t fd = *(int32_t *)data;
		if (fd >= 0)
			return vfio_virqfd_enable((void *) vdev,
						  vfio_pci_intx_unmask_handler,
						  vfio_send_intx_eventfd, NULL,
						  &vdev->ctx[0].unmask, fd);

		vfio_virqfd_disable(&vdev->ctx[0].unmask);
	}

	return 0;
}

static int vfio_pci_set_intx_mask(struct vfio_pci_device *vdev,
				  unsigned index, unsigned start,
				  unsigned count, uint32_t flags, void *data)
{
	if (!is_intx(vdev) || start != 0 || count != 1)
		return -EINVAL;

	if (flags & VFIO_IRQ_SET_DATA_NONE) {
		vfio_pci_intx_mask(vdev);
	} else if (flags & VFIO_IRQ_SET_DATA_BOOL) {
		uint8_t mask = *(uint8_t *)data;
		if (mask)
			vfio_pci_intx_mask(vdev);
	} else if (flags & VFIO_IRQ_SET_DATA_EVENTFD) {
		return -ENOTTY; /* XXX implement me */
	}

	return 0;
}

static int vfio_pci_set_intx_trigger(struct vfio_pci_device *vdev,
				     unsigned index, unsigned start,
				     unsigned count, uint32_t flags, void *data)
{
	if (is_intx(vdev) && !count && (flags & VFIO_IRQ_SET_DATA_NONE)) {
		vfio_intx_disable(vdev);
		return 0;
	}

	if (!(is_intx(vdev) || is_irq_none(vdev)) || start != 0 || count != 1)
		return -EINVAL;

	if (flags & VFIO_IRQ_SET_DATA_EVENTFD) {
		int32_t fd = *(int32_t *)data;
		int ret;

		if (is_intx(vdev))
			return vfio_intx_set_signal(vdev, fd);

		ret = vfio_intx_enable(vdev);
		if (ret)
			return ret;

		ret = vfio_intx_set_signal(vdev, fd);
		if (ret)
			vfio_intx_disable(vdev);

		return ret;
	}

	if (!is_intx(vdev))
		return -EINVAL;

	if (flags & VFIO_IRQ_SET_DATA_NONE) {
		vfio_send_intx_eventfd(vdev, NULL);
	} else if (flags & VFIO_IRQ_SET_DATA_BOOL) {
		uint8_t trigger = *(uint8_t *)data;
		if (trigger)
			vfio_send_intx_eventfd(vdev, NULL);
	}
	return 0;
}

static int vfio_pci_set_msi_trigger(struct vfio_pci_device *vdev,
				    unsigned index, unsigned start,
				    unsigned count, uint32_t flags, void *data)
{
	int i;
	bool msix = (index == VFIO_PCI_MSIX_IRQ_INDEX) ? true : false;

	if (irq_is(vdev, index) && !count && (flags & VFIO_IRQ_SET_DATA_NONE)) {
		vfio_msi_disable(vdev, msix);
		return 0;
	}

	if (!(irq_is(vdev, index) || is_irq_none(vdev)))
		return -EINVAL;

	if (flags & VFIO_IRQ_SET_DATA_EVENTFD) {
		int32_t *fds = data;
		int ret;

		if (vdev->irq_type == index)
			return vfio_msi_set_block(vdev, start, count,
						  fds, msix);

		ret = vfio_msi_enable(vdev, start + count, msix);
		if (ret)
			return ret;

		ret = vfio_msi_set_block(vdev, start, count, fds, msix);
		if (ret)
			vfio_msi_disable(vdev, msix);

		return ret;
	}

	if (!irq_is(vdev, index) || start + count > vdev->num_ctx)
		return -EINVAL;

	for (i = start; i < start + count; i++) {
		if (!vdev->ctx[i].trigger)
			continue;
		if (flags & VFIO_IRQ_SET_DATA_NONE) {
			eventfd_signal(vdev->ctx[i].trigger, 1);
		} else if (flags & VFIO_IRQ_SET_DATA_BOOL) {
			uint8_t *bools = data;
			if (bools[i - start])
				eventfd_signal(vdev->ctx[i].trigger, 1);
		}
	}
	return 0;
}

static int vfio_pci_set_ctx_trigger_single(struct eventfd_ctx **ctx,
					   unsigned int count, uint32_t flags,
					   void *data)
{
	/* DATA_NONE/DATA_BOOL enables loopback testing */
	if (flags & VFIO_IRQ_SET_DATA_NONE) {
		if (*ctx) {
			if (count) {
				eventfd_signal(*ctx, 1);
			} else {
				eventfd_ctx_put(*ctx);
				*ctx = NULL;
			}
			return 0;
		}
	} else if (flags & VFIO_IRQ_SET_DATA_BOOL) {
		uint8_t trigger;

		if (!count)
			return -EINVAL;

		trigger = *(uint8_t *)data;
		if (trigger && *ctx)
			eventfd_signal(*ctx, 1);

		return 0;
	} else if (flags & VFIO_IRQ_SET_DATA_EVENTFD) {
		int32_t fd;

		if (!count)
			return -EINVAL;

		fd = *(int32_t *)data;
		if (fd == -1) {
			if (*ctx)
				eventfd_ctx_put(*ctx);
			*ctx = NULL;
		} else if (fd >= 0) {
			struct eventfd_ctx *efdctx;

			efdctx = eventfd_ctx_fdget(fd);
			if (IS_ERR(efdctx))
				return PTR_ERR(efdctx);

			if (*ctx)
				eventfd_ctx_put(*ctx);

			*ctx = efdctx;
		}
		return 0;
	}

	return -EINVAL;
}

static int vfio_pci_set_err_trigger(struct vfio_pci_device *vdev,
				    unsigned index, unsigned start,
				    unsigned count, uint32_t flags, void *data)
{
	if (index != VFIO_PCI_ERR_IRQ_INDEX || start != 0 || count > 1)
		return -EINVAL;

	return vfio_pci_set_ctx_trigger_single(&vdev->err_trigger,
					       count, flags, data);
}

static int vfio_pci_set_req_trigger(struct vfio_pci_device *vdev,
				    unsigned index, unsigned start,
				    unsigned count, uint32_t flags, void *data)
{
	if (index != VFIO_PCI_REQ_IRQ_INDEX || start != 0 || count > 1)
		return -EINVAL;

	return vfio_pci_set_ctx_trigger_single(&vdev->req_trigger,
					       count, flags, data);
}

int vfio_pci_set_irqs_ioctl(struct vfio_pci_device *vdev, uint32_t flags,
			    unsigned index, unsigned start, unsigned count,
			    void *data)
{
	int (*func)(struct vfio_pci_device *vdev, unsigned index,
		    unsigned start, unsigned count, uint32_t flags,
		    void *data) = NULL;

	switch (index) {
	case VFIO_PCI_INTX_IRQ_INDEX:
		switch (flags & VFIO_IRQ_SET_ACTION_TYPE_MASK) {
		case VFIO_IRQ_SET_ACTION_MASK:
			func = vfio_pci_set_intx_mask;
			break;
		case VFIO_IRQ_SET_ACTION_UNMASK:
			func = vfio_pci_set_intx_unmask;
			break;
		case VFIO_IRQ_SET_ACTION_TRIGGER:
			func = vfio_pci_set_intx_trigger;
			break;
		}
		break;
	case VFIO_PCI_MSI_IRQ_INDEX:
	case VFIO_PCI_MSIX_IRQ_INDEX:
		switch (flags & VFIO_IRQ_SET_ACTION_TYPE_MASK) {
		case VFIO_IRQ_SET_ACTION_MASK:
		case VFIO_IRQ_SET_ACTION_UNMASK:
			/* XXX Need masking support exported */
			break;
		case VFIO_IRQ_SET_ACTION_TRIGGER:
			func = vfio_pci_set_msi_trigger;
			break;
		}
		break;
	case VFIO_PCI_ERR_IRQ_INDEX:
		switch (flags & VFIO_IRQ_SET_ACTION_TYPE_MASK) {
		case VFIO_IRQ_SET_ACTION_TRIGGER:
			if (pci_is_pcie(vdev->pdev))
				func = vfio_pci_set_err_trigger;
			break;
		}
		break;
	case VFIO_PCI_REQ_IRQ_INDEX:
		switch (flags & VFIO_IRQ_SET_ACTION_TYPE_MASK) {
		case VFIO_IRQ_SET_ACTION_TRIGGER:
			func = vfio_pci_set_req_trigger;
			break;
		}
		break;
	}

	if (!func)
		return -ENOTTY;

	return func(vdev, index, start, count, flags, data);
}