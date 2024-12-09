int gdImagePaletteToTrueColor(gdImagePtr src)
{
	unsigned int y;
	unsigned char alloc_y = 0;
	unsigned int yy;

	if (src == NULL) {
		return 0;