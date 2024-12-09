	if (mode->hsync)
		return mode->hsync;

	if (mode->htotal <= 0)
		return 0;

	calc_val = (mode->clock * 1000) / mode->htotal; /* hsync in Hz */
	calc_val += 500;				/* round to 1000Hz */