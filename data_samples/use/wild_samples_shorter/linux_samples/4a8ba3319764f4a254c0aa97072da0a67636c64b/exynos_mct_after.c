	writel_relaxed(value, reg_base + offset);

	if (likely(offset >= EXYNOS4_MCT_L_BASE(0))) {
		stat_addr = (offset & EXYNOS4_MCT_L_MASK) + MCT_L_WSTAT_OFFSET;
		switch (offset & ~EXYNOS4_MCT_L_MASK) {
		case MCT_L_TCON_OFFSET:
			mask = 1 << 3;		/* L_TCON write status */
			break;
		case MCT_L_ICNTB_OFFSET: