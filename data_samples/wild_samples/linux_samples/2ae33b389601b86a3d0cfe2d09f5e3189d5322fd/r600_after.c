	} else {
		if (G_008010_PA_BUSY(tmp) | G_008010_SC_BUSY(tmp) |
		    G_008010_SH_BUSY(tmp) | G_008010_SX_BUSY(tmp) |
		    G_008010_TA03_BUSY(tmp) | G_008010_VGT_BUSY(tmp) |
		    G_008010_DB03_BUSY(tmp) | G_008010_CB03_BUSY(tmp) |
		    G_008010_SPI03_BUSY(tmp) | G_008010_VGT_BUSY_NO_DMA(tmp))
			reset_mask |= RADEON_RESET_GFX;
	}

	if (G_008010_CF_RQ_PENDING(tmp) | G_008010_PF_RQ_PENDING(tmp) |
	    G_008010_CP_BUSY(tmp) | G_008010_CP_COHERENCY_BUSY(tmp))
		reset_mask |= RADEON_RESET_CP;

	if (G_008010_GRBM_EE_BUSY(tmp))
		reset_mask |= RADEON_RESET_GRBM | RADEON_RESET_GFX | RADEON_RESET_CP;

	/* DMA_STATUS_REG */
	tmp = RREG32(DMA_STATUS_REG);
	if (!(tmp & DMA_IDLE))
		reset_mask |= RADEON_RESET_DMA;

	/* SRBM_STATUS */
	tmp = RREG32(R_000E50_SRBM_STATUS);
	if (G_000E50_RLC_RQ_PENDING(tmp) | G_000E50_RLC_BUSY(tmp))
		reset_mask |= RADEON_RESET_RLC;

	if (G_000E50_IH_BUSY(tmp))
		reset_mask |= RADEON_RESET_IH;

	if (G_000E50_SEM_BUSY(tmp))
		reset_mask |= RADEON_RESET_SEM;

	if (G_000E50_GRBM_RQ_PENDING(tmp))
		reset_mask |= RADEON_RESET_GRBM;

	if (G_000E50_VMC_BUSY(tmp))
		reset_mask |= RADEON_RESET_VMC;

	if (G_000E50_MCB_BUSY(tmp) | G_000E50_MCDZ_BUSY(tmp) |
	    G_000E50_MCDY_BUSY(tmp) | G_000E50_MCDX_BUSY(tmp) |
	    G_000E50_MCDW_BUSY(tmp))
		reset_mask |= RADEON_RESET_MC;

	if (r600_is_display_hung(rdev))
		reset_mask |= RADEON_RESET_DISPLAY;

	/* Skip MC reset as it's mostly likely not hung, just busy */
	if (reset_mask & RADEON_RESET_MC) {
		DRM_DEBUG("MC busy: 0x%08X, clearing.\n", reset_mask);
		reset_mask &= ~RADEON_RESET_MC;
	}