
	vtotal = var->upper_margin + var->lower_margin + var->vsync_len;

	if (!var->pixclock)
		return -EINVAL;
	pixclock = var->pixclock;

	if((var->vmode & FB_VMODE_MASK) == FB_VMODE_NONINTERLACED) {
		vtotal += var->yres;