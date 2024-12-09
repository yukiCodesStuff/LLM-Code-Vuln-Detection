	char *cp, *ncp;
	short *nsp;
	Py_Int32 *nlp;
	int len, size, x, y, newx1, newx2, newy1, newy2, nlen;
	int ix, iy, xstep, ystep;
	PyObject *rv;

	if ( !PyArg_ParseTuple(args, "s#iiiiiii", &cp, &len, &size, &x, &y,
		PyErr_SetString(ImageopError, "Size should be 1, 2 or 4");
		return 0;
	}
	if (( len != size*x*y ) ||
            ( size != ((len / x) / y) )) {
		PyErr_SetString(ImageopError, "String has incorrect length");
		return 0;
	}
	xstep = (newx1 < newx2)? 1 : -1;
	ystep = (newy1 < newy2)? 1 : -1;
    
        nlen = (abs(newx2-newx1)+1)*(abs(newy2-newy1)+1)*size;
        if ( size != ((nlen / (abs(newx2-newx1)+1)) / (abs(newy2-newy1)+1)) ) {
		PyErr_SetString(ImageopError, "String has incorrect length");
		return 0;
	}
	rv = PyString_FromStringAndSize(NULL,
			     (abs(newx2-newx1)+1)*(abs(newy2-newy1)+1)*size);
	if ( rv == 0 )
		return 0;
	char *cp, *ncp;
	short *nsp;
	Py_Int32 *nlp;
	int len, size, x, y, newx, newy, nlen;
	int ix, iy;
	int oix, oiy;
	PyObject *rv;

		PyErr_SetString(ImageopError, "Size should be 1, 2 or 4");
		return 0;
	}
	if ( ( len != size*x*y ) ||
             ( size != ((len / x) / y) ) ) {
		PyErr_SetString(ImageopError, "String has incorrect length");
		return 0;
	}
        nlen = newx*newy*size;
	if ( size != ((nlen / newx) / newy) ) {
		PyErr_SetString(ImageopError, "String has incorrect length");
		return 0;
	}
    
	rv = PyString_FromStringAndSize(NULL, nlen);
	if ( rv == 0 )
		return 0;
	ncp = (char *)PyString_AsString(rv);
	nsp = (short *)ncp;
		PyErr_SetString(ImageopError, "Size should be 1 or 4");
		return 0;
	}
	if ( ( maxx*maxy*width != len ) ||
             ( maxx != ((len / maxy) / width) ) ) {
		PyErr_SetString(ImageopError, "String has incorrect length");
		return 0;
	}
    
	if ( !PyArg_ParseTuple(args, "s#iii", &cp, &len, &x, &y, &tres) )
		return 0;

	if ( ( x*y != len ) ||
             ( x != len / y ) ) {
		PyErr_SetString(ImageopError, "String has incorrect length");
		return 0;
	}
    
	if ( !PyArg_ParseTuple(args, "s#ii", &cp, &len, &x, &y) )
		return 0;

	if ( ( x*y != len ) ||
             ( x != len / y ) ) {
		PyErr_SetString(ImageopError, "String has incorrect length");
		return 0;
	}
    
	if ( !PyArg_ParseTuple(args, "s#ii", &cp, &len, &x, &y) )
		return 0;

	if ( ( x*y != len ) ||
             ( x != len / y ) ) {
		PyErr_SetString(ImageopError, "String has incorrect length");
		return 0;
	}
    
	if ( !PyArg_ParseTuple(args, "s#ii", &cp, &len, &x, &y) )
		return 0;

	if ( ( x*y != len ) ||
             ( x != len / y ) ) {
		PyErr_SetString(ImageopError, "String has incorrect length");
		return 0;
	}
    
	if ( !PyArg_ParseTuple(args, "s#ii", &cp, &len, &x, &y) )
		return 0;

	if ( ( x*y != len ) ||
             ( x != len / y ) ) {
		PyErr_SetString(ImageopError, "String has incorrect length");
		return 0;
	}
    
	if ( !PyArg_ParseTuple(args, "s#iiii", &cp, &len, &x, &y, &v0, &v1) )
		return 0;

        nlen = x*y;
	if ( x != (nlen / y) ) {
		PyErr_SetString(ImageopError, "String has incorrect length");
		return 0;
	}
	if ( (nlen+7)/8 != len ) {
		PyErr_SetString(ImageopError, "String has incorrect length");
		return 0;
	}
		return 0;

	nlen = x*y;
	if ( x != (nlen / y) ) {
		PyErr_SetString(ImageopError, "String has incorrect length");
		return 0;
	}
	if ( (nlen+3)/4 != len ) {
		PyErr_SetString(ImageopError, "String has incorrect length");
		return 0;
	}
		return 0;

	nlen = x*y;
	if ( x != (nlen / y) ) {
		PyErr_SetString(ImageopError, "String has incorrect length");
		return 0;
	}
	if ( (nlen+1)/2 != len ) {
		PyErr_SetString(ImageopError, "String has incorrect length");
		return 0;
	}
		return 0;

	nlen = x*y;
	if ( x != (nlen / y) ) {
		PyErr_SetString(ImageopError, "String has incorrect length");
		return 0;
	}
	if ( nlen*4 != len ) {
		PyErr_SetString(ImageopError, "String has incorrect length");
		return 0;
	}
		return 0;

	nlen = x*y;
	if ( x != (nlen / y) ) {
		PyErr_SetString(ImageopError, "String has incorrect length");
		return 0;
	}
	if ( nlen != len ) {
		PyErr_SetString(ImageopError, "String has incorrect length");
		return 0;
	}
	
	if ( nlen / x != y || nlen > INT_MAX / 4) {
		PyErr_SetString(ImageopError, "Image is too large");
		return 0;
	}
    
	rv = PyString_FromStringAndSize(NULL, nlen*4);
	if ( rv == 0 )
		return 0;
		return 0;

	nlen = x*y;
	if ( x != (nlen / y) ) {
		PyErr_SetString(ImageopError, "String has incorrect length");
		return 0;
	}
	if ( nlen*4 != len ) {
		PyErr_SetString(ImageopError, "String has incorrect length");
		return 0;
	}
		return 0;

	nlen = x*y;
	if ( x != (nlen / y) ) {
		PyErr_SetString(ImageopError, "String has incorrect length");
		return 0;
	}
	if ( nlen != len ) {
		PyErr_SetString(ImageopError, "String has incorrect length");
		return 0;
	}
	
	if ( nlen / x != y || nlen > INT_MAX / 4) {
		PyErr_SetString(ImageopError, "Image is too large");
		return 0;
	}
    
	rv = PyString_FromStringAndSize(NULL, nlen*4);
	if ( rv == 0 )
		return 0;