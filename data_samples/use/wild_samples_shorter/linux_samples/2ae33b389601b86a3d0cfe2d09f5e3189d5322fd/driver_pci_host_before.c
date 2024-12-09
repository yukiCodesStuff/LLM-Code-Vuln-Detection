		return;
	}

	pc->host_controller = pc_host;
	pc_host->pci_controller.io_resource = &pc_host->io_resource;
	pc_host->pci_controller.mem_resource = &pc_host->mem_resource;
	pc_host->pci_controller.pci_ops = &pc_host->pci_ops;