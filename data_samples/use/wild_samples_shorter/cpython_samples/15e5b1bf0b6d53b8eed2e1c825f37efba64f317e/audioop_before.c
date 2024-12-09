
static PyObject *AudioopError;

static PyObject *
audioop_getsample(PyObject *self, PyObject *args)
{
    signed char *cp;

    if ( !PyArg_ParseTuple(args, "s#ii:getsample", &cp, &len, &size, &i) )
        return 0;
    if ( size != 1 && size != 2 && size != 4 ) {
        PyErr_SetString(AudioopError, "Size should be 1, 2 or 4");
        return 0;
    }
    if ( i < 0 || i >= len/size ) {
        PyErr_SetString(AudioopError, "Index out of range");
        return 0;
    }

    if ( !PyArg_ParseTuple(args, "s#i:max", &cp, &len, &size) )
        return 0;
    if ( size != 1 && size != 2 && size != 4 ) {
        PyErr_SetString(AudioopError, "Size should be 1, 2 or 4");
        return 0;
    }
    for ( i=0; i<len; i+= size) {
        if ( size == 1 )      val = (int)*CHARP(cp, i);
        else if ( size == 2 ) val = (int)*SHORTP(cp, i);
        else if ( size == 4 ) val = (int)*LONGP(cp, i);

    if (!PyArg_ParseTuple(args, "s#i:minmax", &cp, &len, &size))
        return NULL;
    if (size != 1 && size != 2 && size != 4) {
        PyErr_SetString(AudioopError, "Size should be 1, 2 or 4");
        return NULL;
    }
    for (i = 0; i < len; i += size) {
        if (size == 1) val = (int) *CHARP(cp, i);
        else if (size == 2) val = (int) *SHORTP(cp, i);
        else if (size == 4) val = (int) *LONGP(cp, i);

    if ( !PyArg_ParseTuple(args, "s#i:avg", &cp, &len, &size) )
        return 0;
    if ( size != 1 && size != 2 && size != 4 ) {
        PyErr_SetString(AudioopError, "Size should be 1, 2 or 4");
        return 0;
    }
    for ( i=0; i<len; i+= size) {
        if ( size == 1 )      val = (int)*CHARP(cp, i);
        else if ( size == 2 ) val = (int)*SHORTP(cp, i);
        else if ( size == 4 ) val = (int)*LONGP(cp, i);

    if ( !PyArg_ParseTuple(args, "s#i:rms", &cp, &len, &size) )
        return 0;
    if ( size != 1 && size != 2 && size != 4 ) {
        PyErr_SetString(AudioopError, "Size should be 1, 2 or 4");
        return 0;
    }
    for ( i=0; i<len; i+= size) {
        if ( size == 1 )      val = (int)*CHARP(cp, i);
        else if ( size == 2 ) val = (int)*SHORTP(cp, i);
        else if ( size == 4 ) val = (int)*LONGP(cp, i);

    if ( !PyArg_ParseTuple(args, "s#i:avgpp", &cp, &len, &size) )
        return 0;
    if ( size != 1 && size != 2 && size != 4 ) {
        PyErr_SetString(AudioopError, "Size should be 1, 2 or 4");
        return 0;
    }
    /* Compute first delta value ahead. Also automatically makes us
    ** skip the first extreme value
    */
    if ( size == 1 )      prevval = (int)*CHARP(cp, 0);

    if ( !PyArg_ParseTuple(args, "s#i:maxpp", &cp, &len, &size) )
        return 0;
    if ( size != 1 && size != 2 && size != 4 ) {
        PyErr_SetString(AudioopError, "Size should be 1, 2 or 4");
        return 0;
    }
    /* Compute first delta value ahead. Also automatically makes us
    ** skip the first extreme value
    */
    if ( size == 1 )      prevval = (int)*CHARP(cp, 0);

    if ( !PyArg_ParseTuple(args, "s#i:cross", &cp, &len, &size) )
        return 0;
    if ( size != 1 && size != 2 && size != 4 ) {
        PyErr_SetString(AudioopError, "Size should be 1, 2 or 4");
        return 0;
    }
    ncross = -1;
    prevval = 17; /* Anything <> 0,1 */
    for ( i=0; i<len; i+= size) {
        if ( size == 1 )      val = ((int)*CHARP(cp, i)) >> 7;

    if ( !PyArg_ParseTuple(args, "s#id:mul", &cp, &len, &size, &factor ) )
        return 0;

    if ( size == 1 ) maxval = (double) 0x7f;
    else if ( size == 2 ) maxval = (double) 0x7fff;
    else if ( size == 4 ) maxval = (double) 0x7fffffff;
    if ( !PyArg_ParseTuple(args, "s#idd:tomono",
                           &cp, &len, &size, &fac1, &fac2 ) )
        return 0;

    if ( size == 1 ) maxval = (double) 0x7f;
    else if ( size == 2 ) maxval = (double) 0x7fff;
    else if ( size == 4 ) maxval = (double) 0x7fffffff;
    if ( !PyArg_ParseTuple(args, "s#idd:tostereo",
                           &cp, &len, &size, &fac1, &fac2 ) )
        return 0;

    if ( size == 1 ) maxval = (double) 0x7f;
    else if ( size == 2 ) maxval = (double) 0x7fff;
    else if ( size == 4 ) maxval = (double) 0x7fffffff;
    if ( !PyArg_ParseTuple(args, "s#s#i:add",
                      &cp1, &len1, &cp2, &len2, &size ) )
        return 0;

    if ( len1 != len2 ) {
        PyErr_SetString(AudioopError, "Lengths should be the same");
        return 0;
    }
                      &cp, &len, &size , &bias) )
        return 0;

    if ( size != 1 && size != 2 && size != 4) {
        PyErr_SetString(AudioopError, "Size should be 1, 2 or 4");
        return 0;
    }

    rv = PyString_FromStringAndSize(NULL, len);
    if ( rv == 0 )
        return 0;
                      &cp, &len, &size) )
        return 0;

    if ( size != 1 && size != 2 && size != 4 ) {
        PyErr_SetString(AudioopError, "Size should be 1, 2 or 4");
        return 0;
    }

    rv = PyString_FromStringAndSize(NULL, len);
    if ( rv == 0 )
        return 0;
                      &cp, &len, &size, &size2) )
        return 0;

    if ( (size != 1 && size != 2 && size != 4) ||
         (size2 != 1 && size2 != 2 && size2 != 4)) {
        PyErr_SetString(AudioopError, "Size should be 1, 2 or 4");
        return 0;
    }

    if (len/size > INT_MAX/size2) {
        PyErr_SetString(PyExc_MemoryError,
                        "not enough memory for output buffer");
                          &nchannels, &inrate, &outrate, &state,
                          &weightA, &weightB))
        return NULL;
    if (size != 1 && size != 2 && size != 4) {
        PyErr_SetString(AudioopError, "Size should be 1, 2 or 4");
        return NULL;
    }
    if (nchannels < 1) {
        PyErr_SetString(AudioopError, "# of channels should be >= 1");
        return NULL;
    }
                           &cp, &len, &size) )
        return 0 ;

    if ( size != 1 && size != 2 && size != 4) {
        PyErr_SetString(AudioopError, "Size should be 1, 2 or 4");
        return 0;
    }

    rv = PyString_FromStringAndSize(NULL, len/size);
    if ( rv == 0 )
        return 0;
                           &cp, &len, &size) )
        return 0;

    if ( size != 1 && size != 2 && size != 4) {
        PyErr_SetString(AudioopError, "Size should be 1, 2 or 4");
        return 0;
    }

    if (len > INT_MAX/size) {
        PyErr_SetString(PyExc_MemoryError,
                        "not enough memory for output buffer");
                           &cp, &len, &size) )
        return 0;

    if ( size != 1 && size != 2 && size != 4) {
        PyErr_SetString(AudioopError, "Size should be 1, 2 or 4");
        return 0;
    }

    rv = PyString_FromStringAndSize(NULL, len/size);
    if ( rv == 0 )
        return 0;
                           &cp, &len, &size) )
        return 0;

    if ( size != 1 && size != 2 && size != 4) {
        PyErr_SetString(AudioopError, "Size should be 1, 2 or 4");
        return 0;
    }

    if (len > INT_MAX/size) {
        PyErr_SetString(PyExc_MemoryError,
                        "not enough memory for output buffer");
                           &cp, &len, &size, &state) )
        return 0;


    if ( size != 1 && size != 2 && size != 4) {
        PyErr_SetString(AudioopError, "Size should be 1, 2 or 4");
        return 0;
    }

    str = PyString_FromStringAndSize(NULL, len/(size*2));
    if ( str == 0 )
        return 0;
                           &cp, &len, &size, &state) )
        return 0;

    if ( size != 1 && size != 2 && size != 4) {
        PyErr_SetString(AudioopError, "Size should be 1, 2 or 4");
        return 0;
    }

    /* Decode state, should have (value, step) */
    if ( state == Py_None ) {
        /* First time, it seems. Set defaults */