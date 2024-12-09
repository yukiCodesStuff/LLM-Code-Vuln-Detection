			val = swahb32(val);
		}

		iowrite32(val, mem + reg);
		usleep_range(100, 120);
	}

	pci_read_config_word(pdev, PCI_COMMAND, &cmd);