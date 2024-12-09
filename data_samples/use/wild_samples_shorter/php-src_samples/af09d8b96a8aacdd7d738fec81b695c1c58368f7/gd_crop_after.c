	gdImagePtr dst;
	int y;

	/* allocate the requested size (could be only partially filled) */
	if (src->trueColor) {
		dst = gdImageCreateTrueColor(crop->width, crop->height);
		if (dst == NULL) {
			return NULL;
		}
		gdImageSaveAlpha(dst, 1);
	} else {
		dst = gdImageCreate(crop->width, crop->height);
		if (dst == NULL) {
			return NULL;
		}
		gdImagePaletteCopy(dst, src);
	}
	dst->transparent = src->transparent;

	/* check position in the src image */
	if (crop->x < 0 || crop->x>=src->sx || crop->y<0 || crop->y>=src->sy) {