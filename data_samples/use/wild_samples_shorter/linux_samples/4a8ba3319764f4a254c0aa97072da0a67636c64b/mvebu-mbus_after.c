}

/* Checks whether the given window number is available */

/* On Armada XP, 375 and 38x the MBus window 13 has the remap
 * capability, like windows 0 to 7. However, the mvebu-mbus driver
 * isn't currently taking into account this special case, which means
 * that when window 13 is actually used, the remap registers are left
 * to 0, making the device using this MBus window unavailable. The
 * quick fix for stable is to not use window 13. A follow up patch
 * will correctly handle this window.
*/
static int mvebu_mbus_window_is_free(struct mvebu_mbus_state *mbus,
				     const int win)
{
	void __iomem *addr = mbus->mbuswins_base +
		mbus->soc->win_cfg_offset(win);
	u32 ctrl = readl(addr + WIN_CTRL_OFF);

	if (win == 13)
		return false;

	return !(ctrl & WIN_CTRL_ENABLE);
}

/*