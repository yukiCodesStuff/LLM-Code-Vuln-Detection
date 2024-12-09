	char *cp, *ncp;
	short *nsp;
	Py_Int32 *nlp;
	int len, size, x, y, newx1, newx2, newy1, newy2;
	int ix, iy, xstep, ystep;
	PyObject *rv;

	if ( !PyArg_ParseTuple(args, "s#iiiiiii", &cp, &len, &size, &x, &y,
		PyErr_SetString(ImageopError, "Size should be 1, 2 or 4");
		return 0;
	}
	if ( len != size*x*y ) {
		PyErr_SetString(ImageopError, "String has incorrect length");
		return 0;
	}
	xstep = (newx1 < newx2)? 1 : -1;
	ystep = (newy1 < newy2)? 1 : -1;
    
	rv = PyString_FromStringAndSize(NULL,
			     (abs(newx2-newx1)+1)*(abs(newy2-newy1)+1)*size);
	if ( rv == 0 )
		return 0;
	char *cp, *ncp;
	short *nsp;
	Py_Int32 *nlp;
	int len, size, x, y, newx, newy;
	int ix, iy;
	int oix, oiy;
	PyObject *rv;

		PyErr_SetString(ImageopError, "Size should be 1, 2 or 4");
		return 0;
	}
	if ( len != size*x*y ) {
		PyErr_SetString(ImageopError, "String has incorrect length");
		return 0;
	}
    
	rv = PyString_FromStringAndSize(NULL, newx*newy*size);
	if ( rv == 0 )
		return 0;
	ncp = (char *)PyString_AsString(rv);
	nsp = (short *)ncp;
		PyErr_SetString(ImageopError, "Size should be 1 or 4");
		return 0;
	}
	if ( maxx*maxy*width != len ) {
		PyErr_SetString(ImageopError, "String has incorrect length");
		return 0;
	}
    
	if ( !PyArg_ParseTuple(args, "s#iii", &cp, &len, &x, &y, &tres) )
		return 0;

	if ( x*y != len ) {
		PyErr_SetString(ImageopError, "String has incorrect length");
		return 0;
	}
    
	if ( !PyArg_ParseTuple(args, "s#ii", &cp, &len, &x, &y) )
		return 0;

	if ( x*y != len ) {
		PyErr_SetString(ImageopError, "String has incorrect length");
		return 0;
	}
    
	if ( !PyArg_ParseTuple(args, "s#ii", &cp, &len, &x, &y) )
		return 0;

	if ( x*y != len ) {
		PyErr_SetString(ImageopError, "String has incorrect length");
		return 0;
	}
    
	if ( !PyArg_ParseTuple(args, "s#ii", &cp, &len, &x, &y) )
		return 0;

	if ( x*y != len ) {
		PyErr_SetString(ImageopError, "String has incorrect length");
		return 0;
	}
    
	if ( !PyArg_ParseTuple(args, "s#ii", &cp, &len, &x, &y) )
		return 0;

	if ( x*y != len ) {
		PyErr_SetString(ImageopError, "String has incorrect length");
		return 0;
	}
    
	if ( !PyArg_ParseTuple(args, "s#iiii", &cp, &len, &x, &y, &v0, &v1) )
		return 0;

	nlen = x*y;
	if ( (nlen+7)/8 != len ) {
		PyErr_SetString(ImageopError, "String has incorrect length");
		return 0;
	}
		return 0;

	nlen = x*y;
	if ( (nlen+3)/4 != len ) {
		PyErr_SetString(ImageopError, "String has incorrect length");
		return 0;
	}
		return 0;

	nlen = x*y;
	if ( (nlen+1)/2 != len ) {
		PyErr_SetString(ImageopError, "String has incorrect length");
		return 0;
	}
		return 0;

	nlen = x*y;
	if ( nlen*4 != len ) {
		PyErr_SetString(ImageopError, "String has incorrect length");
		return 0;
	}
		return 0;

	nlen = x*y;
	if ( nlen != len ) {
		PyErr_SetString(ImageopError, "String has incorrect length");
		return 0;
	}
    
	rv = PyString_FromStringAndSize(NULL, nlen*4);
	if ( rv == 0 )
		return 0;
		return 0;

	nlen = x*y;
	if ( nlen*4 != len ) {
		PyErr_SetString(ImageopError, "String has incorrect length");
		return 0;
	}
		return 0;

	nlen = x*y;
	if ( nlen != len ) {
		PyErr_SetString(ImageopError, "String has incorrect length");
		return 0;
	}
    
	rv = PyString_FromStringAndSize(NULL, nlen*4);
	if ( rv == 0 )
		return 0;