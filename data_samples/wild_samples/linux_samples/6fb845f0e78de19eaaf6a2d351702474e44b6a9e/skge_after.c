{
	const struct skge_port *skge = netdev_priv(dev);
	const void __iomem *io = skge->hw->regs;

	regs->version = 1;
	memset(p, 0, regs->len);
	memcpy_fromio(p, io, B3_RAM_ADDR);

	if (regs->len > B3_RI_WTO_R1) {
		memcpy_fromio(p + B3_RI_WTO_R1, io + B3_RI_WTO_R1,
			      regs->len - B3_RI_WTO_R1);
	}