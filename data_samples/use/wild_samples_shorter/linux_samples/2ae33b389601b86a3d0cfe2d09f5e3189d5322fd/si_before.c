	if (tmp & L2_BUSY)
		reset_mask |= RADEON_RESET_VMC;

	return reset_mask;
}

static void si_gpu_soft_reset(struct radeon_device *rdev, u32 reset_mask)