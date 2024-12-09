	} else /* remote PCI bus */
		base = cnspci->cfg1_regs + ((busno & 0xf) << 20);

	return base + (where & 0xffc) + (devfn << 12);
}

static int cns3xxx_pci_read_config(struct pci_bus *bus, unsigned int devfn,
				   int where, int size, u32 *val)
	u32 mask = (0x1ull << (size * 8)) - 1;
	int shift = (where % 4) * 8;

	ret = pci_generic_config_read32(bus, devfn, where, size, val);

	if (ret == PCIBIOS_SUCCESSFUL && !bus->number && !devfn &&
	    (where & 0xffc) == PCI_CLASS_REVISION)
		/*