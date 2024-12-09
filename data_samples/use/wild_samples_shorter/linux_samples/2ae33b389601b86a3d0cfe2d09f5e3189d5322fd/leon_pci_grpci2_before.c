#define CAP9_IOMAP_OFS 0x20
#define CAP9_BARSIZE_OFS 0x24

struct grpci2_priv {
	struct leon_pci_info	info; /* must be on top of this structure */
	struct grpci2_regs	*regs;
	char			irq;
	if (where & 0x3)
		return -EINVAL;

	if (bus == 0 && PCI_SLOT(devfn) != 0)
		devfn += (0x8 * 6);

	/* Select bus */
	spin_lock_irqsave(&grpci2_dev_lock, flags);
	REGSTORE(priv->regs->ctrl, (REGLOAD(priv->regs->ctrl) & ~(0xff << 16)) |
	if (where & 0x3)
		return -EINVAL;

	if (bus == 0 && PCI_SLOT(devfn) != 0)
		devfn += (0x8 * 6);

	/* Select bus */
	spin_lock_irqsave(&grpci2_dev_lock, flags);
	REGSTORE(priv->regs->ctrl, (REGLOAD(priv->regs->ctrl) & ~(0xff << 16)) |
	unsigned int busno = bus->number;
	int ret;

	if (PCI_SLOT(devfn) > 15 || (PCI_SLOT(devfn) == 0 && busno == 0)) {
		*val = ~0;
		return 0;
	}

	struct grpci2_priv *priv = grpci2priv;
	unsigned int busno = bus->number;

	if (PCI_SLOT(devfn) > 15 || (PCI_SLOT(devfn) == 0 && busno == 0))
		return 0;

#ifdef GRPCI2_DEBUG_CFGACCESS
	printk(KERN_INFO "grpci2_write_config: [%02x:%02x:%x] ofs=%d size=%d "
		REGSTORE(regs->ahbmst_map[i], priv->pci_area);

	/* Get the GRPCI2 Host PCI ID */
	grpci2_cfg_r32(priv, 0, 0, PCI_VENDOR_ID, &priv->pciid);

	/* Get address to first (always defined) capability structure */
	grpci2_cfg_r8(priv, 0, 0, PCI_CAPABILITY_LIST, &capptr);

	/* Enable/Disable Byte twisting */
	grpci2_cfg_r32(priv, 0, 0, capptr+CAP9_IOMAP_OFS, &io_map);
	io_map = (io_map & ~0x1) | (priv->bt_enabled ? 1 : 0);
	grpci2_cfg_w32(priv, 0, 0, capptr+CAP9_IOMAP_OFS, io_map);

	/* Setup the Host's PCI Target BARs for other peripherals to access,
	 * and do DMA to the host's memory. The target BARs can be sized and
	 * enabled individually.
				pciadr = 0;
			}
		}
		grpci2_cfg_w32(priv, 0, 0, capptr+CAP9_BARSIZE_OFS+i*4, bar_sz);
		grpci2_cfg_w32(priv, 0, 0, PCI_BASE_ADDRESS_0+i*4, pciadr);
		grpci2_cfg_w32(priv, 0, 0, capptr+CAP9_BAR_OFS+i*4, ahbadr);
		printk(KERN_INFO "        TGT BAR[%d]: 0x%08x (PCI)-> 0x%08x\n",
			i, pciadr, ahbadr);
	}

	/* set as bus master and enable pci memory responses */
	grpci2_cfg_r32(priv, 0, 0, PCI_COMMAND, &data);
	data |= (PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER);
	grpci2_cfg_w32(priv, 0, 0, PCI_COMMAND, data);

	/* Enable Error respone (CPU-TRAP) on illegal memory access. */
	REGSTORE(regs->ctrl, CTRL_ER | CTRL_PE);
}