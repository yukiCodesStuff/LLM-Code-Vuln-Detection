{
	const struct skge_port *skge = netdev_priv(dev);
	const void __iomem *io = skge->hw->regs;

	regs->version = 1;
	memset(p, 0, regs->len);
	memcpy_fromio(p, io, B3_RAM_ADDR);

	memcpy_fromio(p + B3_RI_WTO_R1, io + B3_RI_WTO_R1,
		      regs->len - B3_RI_WTO_R1);
}

/* Wake on Lan only supported on Yukon chips with rev 1 or above */
static u32 wol_supported(const struct skge_hw *hw)
{
	if (is_genesis(hw))
		return 0;

	if (hw->chip_id == CHIP_ID_YUKON && hw->chip_rev == 0)
		return 0;

	return WAKE_MAGIC | WAKE_PHY;
}

static void skge_wol_init(struct skge_port *skge)
{
	struct skge_hw *hw = skge->hw;
	int port = skge->port;
	u16 ctrl;

	skge_write16(hw, B0_CTST, CS_RST_CLR);
	skge_write16(hw, SK_REG(port, GMAC_LINK_CTRL), GMLC_RST_CLR);

	/* Turn on Vaux */
	skge_write8(hw, B0_POWER_CTRL,
		    PC_VAUX_ENA | PC_VCC_ENA | PC_VAUX_ON | PC_VCC_OFF);

	/* WA code for COMA mode -- clear PHY reset */
	if (hw->chip_id == CHIP_ID_YUKON_LITE &&
	    hw->chip_rev >= CHIP_REV_YU_LITE_A3) {
		u32 reg = skge_read32(hw, B2_GP_IO);
		reg |= GP_DIR_9;
		reg &= ~GP_IO_9;
		skge_write32(hw, B2_GP_IO, reg);
	}