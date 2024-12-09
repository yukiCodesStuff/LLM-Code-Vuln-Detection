
	DBG("savagefb_check_var");

	if (!var->pixclock)
		return -EINVAL;

	var->transp.offset = 0;
	var->transp.length = 0;
	switch (var->bits_per_pixel) {
	case 8: