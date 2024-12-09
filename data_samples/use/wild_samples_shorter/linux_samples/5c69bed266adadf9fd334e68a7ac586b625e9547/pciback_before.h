static inline void xen_pcibk_release_pci_dev(struct xen_pcibk_device *pdev,
					     struct pci_dev *dev)
{
	if (xen_pcibk_backend && xen_pcibk_backend->free)
		return xen_pcibk_backend->release(pdev, dev);
}

static inline struct pci_dev *