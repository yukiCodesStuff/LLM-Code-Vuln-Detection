	gdImagePtr dst;
	int y;

	/* check size */
	if (crop->width<=0 || crop->height<=0) {
		return NULL;
	}

	/* allocate the requested size (could be only partially filled) */
	if (src->trueColor) {
		dst = gdImageCreateTrueColor(crop->width, crop->height);
		gdImageSaveAlpha(dst, 1);
	} else {
		dst = gdImageCreate(crop->width, crop->height);
		gdImagePaletteCopy(dst, src);
	}
	if (dst == NULL) {
		return NULL;
	}
	dst->transparent = src->transparent;

	/* check position in the src image */
	if (crop->x < 0 || crop->x>=src->sx || crop->y<0 || crop->y>=src->sy) {