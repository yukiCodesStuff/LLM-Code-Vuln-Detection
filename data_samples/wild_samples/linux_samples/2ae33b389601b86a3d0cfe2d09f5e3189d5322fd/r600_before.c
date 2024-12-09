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

	return reset_mask;
}

static void r600_gpu_soft_reset(struct radeon_device *rdev, u32 reset_mask)
{
	struct rv515_mc_save save;
	u32 grbm_soft_reset = 0, srbm_soft_reset = 0;
	u32 tmp;

	if (reset_mask == 0)
		return;

	dev_info(rdev->dev, "GPU softreset: 0x%08X\n", reset_mask);

	r600_print_gpu_status_regs(rdev);

	/* Disable CP parsing/prefetching */
	if (rdev->family >= CHIP_RV770)
		WREG32(R_0086D8_CP_ME_CNTL, S_0086D8_CP_ME_HALT(1) | S_0086D8_CP_PFP_HALT(1));
	else
		WREG32(R_0086D8_CP_ME_CNTL, S_0086D8_CP_ME_HALT(1));

	/* disable the RLC */
	WREG32(RLC_CNTL, 0);

	if (reset_mask & RADEON_RESET_DMA) {
		/* Disable DMA */
		tmp = RREG32(DMA_RB_CNTL);
		tmp &= ~DMA_RB_ENABLE;
		WREG32(DMA_RB_CNTL, tmp);
	}