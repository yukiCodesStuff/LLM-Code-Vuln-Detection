	if (bpp != 1) {
		PyErr_SetString(ImgfileError,
				"image must have 1 byte per pix chan");
		goto finally;
	}
	xsize = image.xsize;
	ysize = image.ysize;
	zsize = image.zsize;
	if (rle) {
		tablen = ysize * zsize * sizeof(Py_Int32);
		rlebuflen = (int) (1.05 * xsize +10);
		if ((tablen / sizeof(Py_Int32)) != (ysize * zsize) ||
		    rlebuflen < 0) {
			PyErr_NoMemory();
			goto finally;
		}

		starttab = (Py_Int32 *)malloc(tablen);
		lengthtab = (Py_Int32 *)malloc(tablen);
		rledat = (unsigned char *)malloc(rlebuflen);
		if (!starttab || !lengthtab || !rledat) {
			PyErr_NoMemory();
			goto finally;
		}
		
		fseek(inf, 512, SEEK_SET);
		readtab(inf, starttab, ysize*zsize);
		readtab(inf, lengthtab, ysize*zsize);

		/* check data order */
		cur = 0;
		badorder = 0;
		for(y = 0; y < ysize; y++) {
			for(z = 0; z < zsize; z++) {
				if (starttab[y + z * ysize] < cur) {
					badorder = 1;
					break;
				}
				cur = starttab[y +z * ysize];
			}
			if (badorder)
				break;
		}

		fseek(inf, 512 + 2 * tablen, SEEK_SET);
		cur = 512 + 2 * tablen;
		new_size = xsize * ysize + TAGLEN;
		if (new_size < 0 || (new_size * sizeof(Py_Int32)) < 0) {
			PyErr_NoMemory();
			goto finally;
		}

		rv = PyString_FromStringAndSize((char *)NULL,
				      new_size * sizeof(Py_Int32));
		if (rv == NULL)
			goto finally;

		base = (unsigned char *) PyString_AsString(rv);
#ifdef ADD_TAGS
		addlongimgtag(base,xsize,ysize);
#endif
		if (badorder) {
			for (z = 0; z < zsize; z++) {
				lptr = base;
				if (reverse_order)
					lptr += (ysize - 1) * xsize
						* sizeof(Py_UInt32);
				for (y = 0; y < ysize; y++) {
					int idx = y + z * ysize;
					if (cur != starttab[idx]) {
						fseek(inf,starttab[idx],
						      SEEK_SET);
						cur = starttab[idx];
					}
					if (lengthtab[idx] > rlebuflen) {
						PyErr_SetString(ImgfileError,
							"rlebuf is too small");
						Py_DECREF(rv);
						rv = NULL;
						goto finally;
					}
					fread(rledat, lengthtab[idx], 1, inf);
					cur += lengthtab[idx];
					expandrow(lptr, rledat, 3-z);
					if (reverse_order)
						lptr -= xsize
						      * sizeof(Py_UInt32);
					else
						lptr += xsize
						      * sizeof(Py_UInt32);
				}
			}
		} else {
			lptr = base;
			if (reverse_order)
				lptr += (ysize - 1) * xsize
					* sizeof(Py_UInt32);
			for (y = 0; y < ysize; y++) {
				for(z = 0; z < zsize; z++) {
					int idx = y + z * ysize;
					if (cur != starttab[idx]) {
						fseek(inf, starttab[idx],
						      SEEK_SET);
						cur = starttab[idx];
					}
					fread(rledat, lengthtab[idx], 1, inf);
					cur += lengthtab[idx];
					expandrow(lptr, rledat, 3-z);
				}
				if (reverse_order)
					lptr -= xsize * sizeof(Py_UInt32);
				else
					lptr += xsize * sizeof(Py_UInt32);
			}
		}
		if (zsize == 3) 
			setalpha(base, xsize * ysize);
		else if (zsize < 3) 
			copybw((Py_Int32 *) base, xsize * ysize);
	}
	else {
		new_size = xsize * ysize + TAGLEN;
		if (new_size < 0 || (new_size * sizeof(Py_Int32)) < 0) {
			PyErr_NoMemory();
			goto finally;
		}

		rv = PyString_FromStringAndSize((char *) 0,
						new_size*sizeof(Py_Int32));
		if (rv == NULL)
			goto finally;

		base = (unsigned char *) PyString_AsString(rv);
#ifdef ADD_TAGS
		addlongimgtag(base, xsize, ysize);
#endif
		verdat = (unsigned char *)malloc(xsize);
		if (!verdat) {
			Py_CLEAR(rv);
			goto finally;
		}

		fseek(inf, 512, SEEK_SET);
		for (z = 0; z < zsize; z++) {
			lptr = base;
			if (reverse_order)
				lptr += (ysize - 1) * xsize
				        * sizeof(Py_UInt32);
			for (y = 0; y < ysize; y++) {
				fread(verdat, xsize, 1, inf);
				interleaverow(lptr, verdat, 3-z, xsize);
				if (reverse_order)
					lptr -= xsize * sizeof(Py_UInt32);
				else
					lptr += xsize * sizeof(Py_UInt32);
			}
		}
		if (zsize == 3)
			setalpha(base, xsize * ysize);
		else if (zsize < 3) 
			copybw((Py_Int32 *) base, xsize * ysize);
	}
  finally:
	if (starttab)
		free(starttab);
	if (lengthtab)
		free(lengthtab);
	if (rledat)
		free(rledat);
	if (verdat)
		free(verdat);
	fclose(inf);
	return rv;
}