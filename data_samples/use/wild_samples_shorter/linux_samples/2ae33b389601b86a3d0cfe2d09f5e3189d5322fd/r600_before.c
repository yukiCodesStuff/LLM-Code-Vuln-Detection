	if (r600_is_display_hung(rdev))
		reset_mask |= RADEON_RESET_DISPLAY;

	return reset_mask;
}

static void r600_gpu_soft_reset(struct radeon_device *rdev, u32 reset_mask)