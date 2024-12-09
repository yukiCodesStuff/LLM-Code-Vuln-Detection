			 struct pci_dev *dev, struct xen_pci_op *op)
{
	struct xen_pcibk_dev_data *dev_data;
	int status;

	if (unlikely(verbose_request))
		printk(KERN_DEBUG DRV_NAME ": %s: enable MSI\n", pci_name(dev));
	status = pci_enable_msi(dev);

	if (status) {
		pr_warn_ratelimited(DRV_NAME ": %s: error enabling MSI for guest %u: err %d\n",
				    pci_name(dev), pdev->xdev->otherend_id,
				    status);
		op->value = 0;
		return XEN_PCI_ERR_op_failed;
	}

						pci_name(dev), i,
						op->msix_entries[i].vector);
		}
	} else
		pr_warn_ratelimited(DRV_NAME ": %s: error enabling MSI-X for guest %u: err %d!\n",
				    pci_name(dev), pdev->xdev->otherend_id,
				    result);
	kfree(entries);

	op->value = result;
	dev_data = pci_get_drvdata(dev);