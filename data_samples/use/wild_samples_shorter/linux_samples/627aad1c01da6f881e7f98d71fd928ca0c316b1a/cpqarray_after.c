		ida_pci_info_struct pciinfo;

		if (!arg) return -EINVAL;
		memset(&pciinfo, 0, sizeof(pciinfo));
		pciinfo.bus = host->pci_dev->bus->number;
		pciinfo.dev_fn = host->pci_dev->devfn;
		pciinfo.board_id = host->board_id;
		if(copy_to_user((void __user *) arg, &pciinfo,  