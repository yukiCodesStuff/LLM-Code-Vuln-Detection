	if (bpp != 1) {
		PyErr_SetString(ImgfileError,
				"image must have 1 byte per pix chan");
		goto finally;
	}
	xsize = image.xsize;
	ysize = image.ysize;
	zsize = image.zsize;
	tablen = xsize * ysize * zsize * sizeof(Py_Int32);
        if (xsize != (((tablen / ysize) / zsize) / sizeof(Py_Int32))) {
		PyErr_NoMemory();
		goto finally;
        }