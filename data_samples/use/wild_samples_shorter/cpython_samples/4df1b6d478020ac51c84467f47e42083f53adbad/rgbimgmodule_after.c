	xsize = image.xsize;
	ysize = image.ysize;
	zsize = image.zsize;
	tablen = xsize * ysize * zsize * sizeof(Py_Int32);
        if (xsize != (((tablen / ysize) / zsize) / sizeof(Py_Int32))) {
		PyErr_NoMemory();
		goto finally;
        }
	if (rle) {
		tablen = ysize * zsize * sizeof(Py_Int32);
		rlebuflen = (int) (1.05 * xsize +10);
		if ((tablen / sizeof(Py_Int32)) != (ysize * zsize) ||