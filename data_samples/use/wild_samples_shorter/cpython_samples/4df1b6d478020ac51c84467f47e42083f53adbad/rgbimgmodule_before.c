	xsize = image.xsize;
	ysize = image.ysize;
	zsize = image.zsize;
	if (rle) {
		tablen = ysize * zsize * sizeof(Py_Int32);
		rlebuflen = (int) (1.05 * xsize +10);
		if ((tablen / sizeof(Py_Int32)) != (ysize * zsize) ||