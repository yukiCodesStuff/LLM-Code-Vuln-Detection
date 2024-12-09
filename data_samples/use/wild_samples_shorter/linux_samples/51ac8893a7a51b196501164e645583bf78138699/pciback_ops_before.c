			 struct pci_dev *dev, struct xen_pci_op *op)
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