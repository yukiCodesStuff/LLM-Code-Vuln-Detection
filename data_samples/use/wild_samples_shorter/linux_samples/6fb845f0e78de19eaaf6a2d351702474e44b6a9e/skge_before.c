	memset(p, 0, regs->len);
	memcpy_fromio(p, io, B3_RAM_ADDR);

	memcpy_fromio(p + B3_RI_WTO_R1, io + B3_RI_WTO_R1,
		      regs->len - B3_RI_WTO_R1);
}

/* Wake on Lan only supported on Yukon chips with rev 1 or above */
static u32 wol_supported(const struct skge_hw *hw)