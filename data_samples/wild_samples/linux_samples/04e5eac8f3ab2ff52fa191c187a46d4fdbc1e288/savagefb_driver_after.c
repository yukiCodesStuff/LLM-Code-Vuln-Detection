{
	struct savagefb_par *par = info->par;
	int memlen, vramlen, mode_valid = 0;

	DBG("savagefb_check_var");

	if (!var->pixclock)
		return -EINVAL;

	var->transp.offset = 0;
	var->transp.length = 0;
	switch (var->bits_per_pixel) {
	case 8:
		var->red.offset = var->green.offset =
			var->blue.offset = 0;
		var->red.length = var->green.length =
			var->blue.length = var->bits_per_pixel;
		break;
	case 16:
		var->red.offset = 11;
		var->red.length = 5;
		var->green.offset = 5;
		var->green.length = 6;
		var->blue.offset = 0;
		var->blue.length = 5;
		break;
	case 32:
		var->transp.offset = 24;
		var->transp.length = 8;
		var->red.offset = 16;
		var->red.length = 8;
		var->green.offset = 8;
		var->green.length = 8;
		var->blue.offset = 0;
		var->blue.length = 8;
		break;

	default:
		return -EINVAL;
	}