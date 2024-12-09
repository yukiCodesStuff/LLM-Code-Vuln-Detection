{
	struct xen_pcibk_dev_data *dev_data;
	int otherend = pdev->xdev->otherend_id;
	int status;

	if (unlikely(verbose_request))
		printk(KERN_DEBUG DRV_NAME ": %s: enable MSI\n", pci_name(dev));

	status = pci_enable_msi(dev);

	if (status) {
		printk(KERN_ERR "error enable msi for guest %x status %x\n",
			otherend, status);
		op->value = 0;
		return XEN_PCI_ERR_op_failed;
	}

	/* The value the guest needs is actually the IDT vector, not the
	 * the local domain's IRQ number. */

	op->value = dev->irq ? xen_pirq_from_irq(dev->irq) : 0;
	if (unlikely(verbose_request))
		printk(KERN_DEBUG DRV_NAME ": %s: MSI: %d\n", pci_name(dev),
			op->value);

	dev_data = pci_get_drvdata(dev);
	if (dev_data)
		dev_data->ack_intr = 0;

	return 0;
}
{
	struct xen_pcibk_dev_data *dev_data;
	int otherend = pdev->xdev->otherend_id;
	int status;

	if (unlikely(verbose_request))
		printk(KERN_DEBUG DRV_NAME ": %s: enable MSI\n", pci_name(dev));

	status = pci_enable_msi(dev);

	if (status) {
		printk(KERN_ERR "error enable msi for guest %x status %x\n",
			otherend, status);
		op->value = 0;
		return XEN_PCI_ERR_op_failed;
	}
		for (i = 0; i < op->value; i++) {
			op->msix_entries[i].entry = entries[i].entry;
			if (entries[i].vector)
				op->msix_entries[i].vector =
					xen_pirq_from_irq(entries[i].vector);
				if (unlikely(verbose_request))
					printk(KERN_DEBUG DRV_NAME ": %s: " \
						"MSI-X[%d]: %d\n",
						pci_name(dev), i,
						op->msix_entries[i].vector);
		}
	} else {
		printk(KERN_WARNING DRV_NAME ": %s: failed to enable MSI-X: err %d!\n",
			pci_name(dev), result);
	}
	kfree(entries);

	op->value = result;
	dev_data = pci_get_drvdata(dev);
	if (dev_data)
		dev_data->ack_intr = 0;

	return result > 0 ? 0 : result;
}

static
int xen_pcibk_disable_msix(struct xen_pcibk_device *pdev,
			   struct pci_dev *dev, struct xen_pci_op *op)
{
	struct xen_pcibk_dev_data *dev_data;
	if (unlikely(verbose_request))
		printk(KERN_DEBUG DRV_NAME ": %s: disable MSI-X\n",
			pci_name(dev));
	pci_disable_msix(dev);

	/*
	 * SR-IOV devices (which don't have any legacy IRQ) have
	 * an undefined IRQ value of zero.
	 */
	op->value = dev->irq ? xen_pirq_from_irq(dev->irq) : 0;
	if (unlikely(verbose_request))
		printk(KERN_DEBUG DRV_NAME ": %s: MSI-X: %d\n", pci_name(dev),
			op->value);
	dev_data = pci_get_drvdata(dev);
	if (dev_data)
		dev_data->ack_intr = 1;
	return 0;
}
#endif
/*
* Now the same evtchn is used for both pcifront conf_read_write request
* as well as pcie aer front end ack. We use a new work_queue to schedule
* xen_pcibk conf_read_write service for avoiding confict with aer_core
* do_recovery job which also use the system default work_queue
*/
void xen_pcibk_test_and_schedule_op(struct xen_pcibk_device *pdev)
{
	/* Check that frontend is requesting an operation and that we are not
	 * already processing a request */
	if (test_bit(_XEN_PCIF_active, (unsigned long *)&pdev->sh_info->flags)
	    && !test_and_set_bit(_PDEVF_op_active, &pdev->flags)) {
		queue_work(xen_pcibk_wq, &pdev->op_work);
	}
	/*_XEN_PCIB_active should have been cleared by pcifront. And also make
	sure xen_pcibk is waiting for ack by checking _PCIB_op_pending*/
	if (!test_bit(_XEN_PCIB_active, (unsigned long *)&pdev->sh_info->flags)
	    && test_bit(_PCIB_op_pending, &pdev->flags)) {
		wake_up(&xen_pcibk_aer_wait_queue);
	}
}

/* Performing the configuration space reads/writes must not be done in atomic
 * context because some of the pci_* functions can sleep (mostly due to ACPI
 * use of semaphores). This function is intended to be called from a work
 * queue in process context taking a struct xen_pcibk_device as a parameter */

void xen_pcibk_do_op(struct work_struct *data)
{
	struct xen_pcibk_device *pdev =
		container_of(data, struct xen_pcibk_device, op_work);
	struct pci_dev *dev;
	struct xen_pcibk_dev_data *dev_data = NULL;
	struct xen_pci_op *op = &pdev->sh_info->op;
	int test_intx = 0;

	dev = xen_pcibk_get_pci_dev(pdev, op->domain, op->bus, op->devfn);

	if (dev == NULL)
		op->err = XEN_PCI_ERR_dev_not_found;
	else {
		dev_data = pci_get_drvdata(dev);
		if (dev_data)
			test_intx = dev_data->enable_intx;
		switch (op->cmd) {
		case XEN_PCI_OP_conf_read:
			op->err = xen_pcibk_config_read(dev,
				  op->offset, op->size, &op->value);
			break;
		case XEN_PCI_OP_conf_write:
			op->err = xen_pcibk_config_write(dev,
				  op->offset, op->size,	op->value);
			break;
#ifdef CONFIG_PCI_MSI
		case XEN_PCI_OP_enable_msi:
			op->err = xen_pcibk_enable_msi(pdev, dev, op);
			break;
		case XEN_PCI_OP_disable_msi:
			op->err = xen_pcibk_disable_msi(pdev, dev, op);
			break;
		case XEN_PCI_OP_enable_msix:
			op->err = xen_pcibk_enable_msix(pdev, dev, op);
			break;
		case XEN_PCI_OP_disable_msix:
			op->err = xen_pcibk_disable_msix(pdev, dev, op);
			break;
#endif
		default:
			op->err = XEN_PCI_ERR_not_implemented;
			break;
		}