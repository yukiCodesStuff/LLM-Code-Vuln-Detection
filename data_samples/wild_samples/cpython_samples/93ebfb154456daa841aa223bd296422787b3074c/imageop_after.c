{
	char *cp, *ncp;
	short *nsp;
	Py_Int32 *nlp;
	int len, size, x, y, newx1, newx2, newy1, newy2, nlen;
	int ix, iy, xstep, ystep;
	PyObject *rv;

	if ( !PyArg_ParseTuple(args, "s#iiiiiii", &cp, &len, &size, &x, &y,
			  &newx1, &newy1, &newx2, &newy2) )
		return 0;
    
	if ( size != 1 && size != 2 && size != 4 ) {
		PyErr_SetString(ImageopError, "Size should be 1, 2 or 4");
		return 0;
	}
	if ( size != 1 && size != 2 && size != 4 ) {
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
	ncp = (char *)PyString_AsString(rv);
	nsp = (short *)ncp;
	nlp = (Py_Int32 *)ncp;
	newy2 += ystep;
	newx2 += xstep;
	for( iy = newy1; iy != newy2; iy+=ystep ) {
		for ( ix = newx1; ix != newx2; ix+=xstep ) {
			if ( iy < 0 || iy >= y || ix < 0 || ix >= x ) {
				if ( size == 1 )
					*ncp++ = 0;
				else
					*nlp++ = 0;
			} else {
				if ( size == 1 )
					*ncp++ = *CHARP(cp, x, ix, iy);
				else if ( size == 2 )
					*nsp++ = *SHORTP(cp, x, ix, iy);
				else
					*nlp++ = *LONGP(cp, x, ix, iy);
			}
		}
	}
{
	char *cp, *ncp;
	short *nsp;
	Py_Int32 *nlp;
	int len, size, x, y, newx, newy, nlen;
	int ix, iy;
	int oix, oiy;
	PyObject *rv;

	if ( !PyArg_ParseTuple(args, "s#iiiii",
			  &cp, &len, &size, &x, &y, &newx, &newy) )
		return 0;
    
	if ( size != 1 && size != 2 && size != 4 ) {
		PyErr_SetString(ImageopError, "Size should be 1, 2 or 4");
		return 0;
	}
	if ( size != 1 && size != 2 && size != 4 ) {
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
	nlp = (Py_Int32 *)ncp;
	for( iy = 0; iy < newy; iy++ ) {
		for ( ix = 0; ix < newx; ix++ ) {
			oix = ix * x / newx;
			oiy = iy * y / newy;
			if ( size == 1 )
				*ncp++ = *CHARP(cp, x, oix, oiy);
			else if ( size == 2 )
				*nsp++ = *SHORTP(cp, x, oix, oiy);
			else
				*nlp++ = *LONGP(cp, x, oix, oiy);
		}
	if ( width != 1 && width != 4 ) {
		PyErr_SetString(ImageopError, "Size should be 1 or 4");
		return 0;
	}
	if ( ( maxx*maxy*width != len ) ||
             ( maxx != ((len / maxy) / width) ) ) {
		PyErr_SetString(ImageopError, "String has incorrect length");
		return 0;
	}
    
	rv = PyString_FromStringAndSize(NULL, len);
	if ( rv == 0 )
		return 0;
	ncp = (unsigned char *)PyString_AsString(rv);

	if ( width == 1 ) {
		memcpy(ncp, cp, maxx);		/* Copy first line */
		ncp += maxx;
		for (y=1; y<maxy; y++) {	/* Interpolate other lines */
			for(x=0; x<maxx; x++) {
				i = y*maxx + x;
				*ncp++ = ((int)cp[i] + (int)cp[i-maxx]) >> 1;
			}
{
	int tres, x, y, len;
	unsigned char *cp, *ncp;
	unsigned char ovalue;
	PyObject *rv;
	int i, bit;
   
    
	if ( !PyArg_ParseTuple(args, "s#iii", &cp, &len, &x, &y, &tres) )
		return 0;

	if ( ( x*y != len ) ||
             ( x != len / y ) ) {
		PyErr_SetString(ImageopError, "String has incorrect length");
		return 0;
	}
{
	int x, y, len;
	unsigned char *cp, *ncp;
	unsigned char ovalue;
	PyObject *rv;
	int i;
	int pos;
   
    
	if ( !PyArg_ParseTuple(args, "s#ii", &cp, &len, &x, &y) )
		return 0;

	if ( ( x*y != len ) ||
             ( x != len / y ) ) {
		PyErr_SetString(ImageopError, "String has incorrect length");
		return 0;
	}
{
	int x, y, len;
	unsigned char *cp, *ncp;
	unsigned char ovalue;
	PyObject *rv;
	int i;
	int pos;
   
    
	if ( !PyArg_ParseTuple(args, "s#ii", &cp, &len, &x, &y) )
		return 0;

	if ( ( x*y != len ) ||
             ( x != len / y ) ) {
		PyErr_SetString(ImageopError, "String has incorrect length");
		return 0;
	}
{
	int sum, x, y, len;
	unsigned char *cp, *ncp;
	unsigned char ovalue;
	PyObject *rv;
	int i, bit;
   
    
	if ( !PyArg_ParseTuple(args, "s#ii", &cp, &len, &x, &y) )
		return 0;

	if ( ( x*y != len ) ||
             ( x != len / y ) ) {
		PyErr_SetString(ImageopError, "String has incorrect length");
		return 0;
	}
{
	int x, y, len;
	unsigned char *cp, *ncp;
	unsigned char ovalue;
	PyObject *rv;
	int i;
	int pos;
	int sum = 0, nvalue;
   
    
	if ( !PyArg_ParseTuple(args, "s#ii", &cp, &len, &x, &y) )
		return 0;

	if ( ( x*y != len ) ||
             ( x != len / y ) ) {
		PyErr_SetString(ImageopError, "String has incorrect length");
		return 0;
	}
{
	int v0, v1, x, y, len, nlen;
	unsigned char *cp, *ncp;
	PyObject *rv;
	int i, bit;
    
	if ( !PyArg_ParseTuple(args, "s#iiii", &cp, &len, &x, &y, &v0, &v1) )
		return 0;

        nlen = x*y;
	if ( x != (nlen / y) ) {
		PyErr_SetString(ImageopError, "String has incorrect length");
		return 0;
	}
{
	int x, y, len, nlen;
	unsigned char *cp, *ncp;
	PyObject *rv;
	int i, pos, value = 0, nvalue;
    
	if ( !PyArg_ParseTuple(args, "s#ii", &cp, &len, &x, &y) )
		return 0;

	nlen = x*y;
	if ( x != (nlen / y) ) {
		PyErr_SetString(ImageopError, "String has incorrect length");
		return 0;
	}
{
	int x, y, len, nlen;
	unsigned char *cp, *ncp;
	PyObject *rv;
	int i, pos, value = 0, nvalue;
    
	if ( !PyArg_ParseTuple(args, "s#ii", &cp, &len, &x, &y) )
		return 0;

	nlen = x*y;
	if ( x != (nlen / y) ) {
		PyErr_SetString(ImageopError, "String has incorrect length");
		return 0;
	}
{
	int x, y, len, nlen;
	unsigned char *cp;
	unsigned char *ncp;
	PyObject *rv;
	int i, r, g, b;
	int backward_compatible = imageop_backward_compatible();
    
	if ( !PyArg_ParseTuple(args, "s#ii", &cp, &len, &x, &y) )
		return 0;

	nlen = x*y;
	if ( x != (nlen / y) ) {
		PyErr_SetString(ImageopError, "String has incorrect length");
		return 0;
	}
{
	int x, y, len, nlen;
	unsigned char *cp;
	unsigned char *ncp;
	PyObject *rv;
	int i, r, g, b;
	unsigned char value;
	int backward_compatible = imageop_backward_compatible();
    
	if ( !PyArg_ParseTuple(args, "s#ii", &cp, &len, &x, &y) )
		return 0;

	nlen = x*y;
	if ( x != (nlen / y) ) {
		PyErr_SetString(ImageopError, "String has incorrect length");
		return 0;
	}
{
	int x, y, len, nlen;
	unsigned char *cp;
	unsigned char *ncp;
	PyObject *rv;
	int i, r, g, b;
	int nvalue;
	int backward_compatible = imageop_backward_compatible();
    
	if ( !PyArg_ParseTuple(args, "s#ii", &cp, &len, &x, &y) )
		return 0;

	nlen = x*y;
	if ( x != (nlen / y) ) {
		PyErr_SetString(ImageopError, "String has incorrect length");
		return 0;
	}
{
	int x, y, len, nlen;
	unsigned char *cp;
	unsigned char *ncp;
	PyObject *rv;
	int i;
	unsigned char value;
	int backward_compatible = imageop_backward_compatible();
    
	if ( !PyArg_ParseTuple(args, "s#ii", &cp, &len, &x, &y) )
		return 0;

	nlen = x*y;
	if ( x != (nlen / y) ) {
		PyErr_SetString(ImageopError, "String has incorrect length");
		return 0;
	}