int gdImagePaletteToTrueColor(gdImagePtr src)
{
	unsigned int y;
	unsigned int yy;

	if (src == NULL) {
		return 0;