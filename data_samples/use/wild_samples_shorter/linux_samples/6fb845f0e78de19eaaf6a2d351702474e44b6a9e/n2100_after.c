/*
 * N2100 PCI.
 */
static int n2100_pci_map_irq(const struct pci_dev *dev, u8 slot, u8 pin)
{
	int irq;

	if (PCI_SLOT(dev->devfn) == 1) {