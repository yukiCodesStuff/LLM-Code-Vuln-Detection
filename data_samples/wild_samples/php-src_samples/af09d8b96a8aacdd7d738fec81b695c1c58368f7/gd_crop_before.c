{
	gdImagePtr dst;
	int y;

	/* check size */
	if (crop->width<=0 || crop->height<=0) {
		return NULL;
	}