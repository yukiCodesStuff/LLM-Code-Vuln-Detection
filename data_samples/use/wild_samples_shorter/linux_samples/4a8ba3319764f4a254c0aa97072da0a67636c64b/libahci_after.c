
	devslp = readl(port_mmio + PORT_DEVSLP);
	if (!(devslp & PORT_DEVSLP_DSP)) {
		dev_info(ap->host->dev, "port does not support device sleep\n");
		return;
	}

	/* disable device sleep */