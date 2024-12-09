{
	struct pci_dev *pdev = vdev->pdev;
	loff_t pos = *ppos & VFIO_PCI_OFFSET_MASK;
	int bar = VFIO_PCI_OFFSET_TO_INDEX(*ppos);
	size_t x_start = 0, x_end = 0;
	resource_size_t end;
	void __iomem *io;
	ssize_t done;

	if (pci_resource_start(pdev, bar))
		end = pci_resource_len(pdev, bar);
	else if (bar == PCI_ROM_RESOURCE &&
		 pdev->resource[bar].flags & IORESOURCE_ROM_SHADOW)
		end = 0x20000;
	else
		return -EINVAL;

	if (pos >= end)
		return -EINVAL;

	count = min(count, (size_t)(end - pos));

	if (bar == PCI_ROM_RESOURCE) {
		/*
		 * The ROM can fill less space than the BAR, so we start the
		 * excluded range at the end of the actual ROM.  This makes
		 * filling large ROM BARs much faster.
		 */
		io = pci_map_rom(pdev, &x_start);
		if (!io)
			return -ENOMEM;
		x_end = end;
	} else {
		int ret = vfio_pci_setup_barmap(vdev, bar);
		if (ret)
			return ret;

		io = vdev->barmap[bar];
	}

	if (bar == vdev->msix_bar) {
		x_start = vdev->msix_offset;
		x_end = vdev->msix_offset + vdev->msix_size;
	}

	done = do_io_rw(io, buf, pos, count, x_start, x_end, iswrite);

	if (done >= 0)
		*ppos += done;

	if (bar == PCI_ROM_RESOURCE)
		pci_unmap_rom(pdev, io);

	return done;
}
{
	struct pci_dev *pdev = vdev->pdev;
	loff_t pos = *ppos & VFIO_PCI_OFFSET_MASK;
	int bar = VFIO_PCI_OFFSET_TO_INDEX(*ppos);
	size_t x_start = 0, x_end = 0;
	resource_size_t end;
	void __iomem *io;
	ssize_t done;

	if (pci_resource_start(pdev, bar))
		end = pci_resource_len(pdev, bar);
	else if (bar == PCI_ROM_RESOURCE &&
		 pdev->resource[bar].flags & IORESOURCE_ROM_SHADOW)
		end = 0x20000;
	else
		return -EINVAL;

	if (pos >= end)
		return -EINVAL;

	count = min(count, (size_t)(end - pos));

	if (bar == PCI_ROM_RESOURCE) {
		/*
		 * The ROM can fill less space than the BAR, so we start the
		 * excluded range at the end of the actual ROM.  This makes
		 * filling large ROM BARs much faster.
		 */
		io = pci_map_rom(pdev, &x_start);
		if (!io)
			return -ENOMEM;
		x_end = end;
	} else {
		int ret = vfio_pci_setup_barmap(vdev, bar);
		if (ret)
			return ret;

		io = vdev->barmap[bar];
	}

	if (bar == vdev->msix_bar) {
		x_start = vdev->msix_offset;
		x_end = vdev->msix_offset + vdev->msix_size;
	}

	done = do_io_rw(io, buf, pos, count, x_start, x_end, iswrite);

	if (done >= 0)
		*ppos += done;

	if (bar == PCI_ROM_RESOURCE)
		pci_unmap_rom(pdev, io);

	return done;
}
	if (bar == vdev->msix_bar) {
		x_start = vdev->msix_offset;
		x_end = vdev->msix_offset + vdev->msix_size;
	}

	done = do_io_rw(io, buf, pos, count, x_start, x_end, iswrite);

	if (done >= 0)
		*ppos += done;

	if (bar == PCI_ROM_RESOURCE)
		pci_unmap_rom(pdev, io);

	return done;
}

ssize_t vfio_pci_vga_rw(struct vfio_pci_device *vdev, char __user *buf,
			       size_t count, loff_t *ppos, bool iswrite)
{
	int ret;
	loff_t off, pos = *ppos & VFIO_PCI_OFFSET_MASK;
	void __iomem *iomem = NULL;
	unsigned int rsrc;
	bool is_ioport;
	ssize_t done;

	if (!vdev->has_vga)
		return -EINVAL;

	if (pos > 0xbfffful)
		return -EINVAL;

	switch ((u32)pos) {
	case 0xa0000 ... 0xbffff:
		count = min(count, (size_t)(0xc0000 - pos));
		iomem = ioremap(0xa0000, 0xbffff - 0xa0000 + 1);
		off = pos - 0xa0000;
		rsrc = VGA_RSRC_LEGACY_MEM;
		is_ioport = false;
		break;
	case 0x3b0 ... 0x3bb:
		count = min(count, (size_t)(0x3bc - pos));
		iomem = ioport_map(0x3b0, 0x3bb - 0x3b0 + 1);
		off = pos - 0x3b0;
		rsrc = VGA_RSRC_LEGACY_IO;
		is_ioport = true;
		break;
	case 0x3c0 ... 0x3df:
		count = min(count, (size_t)(0x3e0 - pos));
		iomem = ioport_map(0x3c0, 0x3df - 0x3c0 + 1);
		off = pos - 0x3c0;
		rsrc = VGA_RSRC_LEGACY_IO;
		is_ioport = true;
		break;
	default:
		return -EINVAL;
	}