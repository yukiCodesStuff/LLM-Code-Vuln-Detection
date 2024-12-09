{
	struct ahci_host_priv *hpriv = ap->host->private_data;
	void __iomem *port_mmio = ahci_port_base(ap);
	struct ata_device *dev = ap->link.device;
	u32 devslp, dm, dito, mdat, deto;
	int rc;
	unsigned int err_mask;

	devslp = readl(port_mmio + PORT_DEVSLP);
	if (!(devslp & PORT_DEVSLP_DSP)) {
		dev_err(ap->host->dev, "port does not support device sleep\n");
		return;
	}