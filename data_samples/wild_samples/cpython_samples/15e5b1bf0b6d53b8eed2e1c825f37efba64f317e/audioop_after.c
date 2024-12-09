static int stepsizeTable[89] = {
    7, 8, 9, 10, 11, 12, 13, 14, 16, 17,
    19, 21, 23, 25, 28, 31, 34, 37, 41, 45,
    50, 55, 60, 66, 73, 80, 88, 97, 107, 118,
    130, 143, 157, 173, 190, 209, 230, 253, 279, 307,
    337, 371, 408, 449, 494, 544, 598, 658, 724, 796,
    876, 963, 1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066,
    2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871, 5358,
    5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899,
    15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767
};

#define CHARP(cp, i) ((signed char *)(cp+i))
#define SHORTP(cp, i) ((short *)(cp+i))
#define LONGP(cp, i) ((Py_Int32 *)(cp+i))



static PyObject *AudioopError;

static int
audioop_check_size(int size)
{
    if (size != 1 && size != 2 && size != 4) {
        PyErr_SetString(AudioopError, "Size should be 1, 2 or 4");
        return 0;
    }
    else
        return 1;
}
{
    signed char *cp;
    int len, size, val = 0;
    int i;

    if ( !PyArg_ParseTuple(args, "s#ii:getsample", &cp, &len, &size, &i) )
        return 0;
    if (!audioop_check_parameters(len, size))
        return NULL;
    if ( i < 0 || i >= len/size ) {
        PyErr_SetString(AudioopError, "Index out of range");
        return 0;
    }
{
    signed char *cp;
    int len, size, val = 0;
    int i;
    int max = 0;

    if ( !PyArg_ParseTuple(args, "s#i:max", &cp, &len, &size) )
        return 0;
    if (!audioop_check_parameters(len, size))
        return NULL;
    for ( i=0; i<len; i+= size) {
        if ( size == 1 )      val = (int)*CHARP(cp, i);
        else if ( size == 2 ) val = (int)*SHORTP(cp, i);
        else if ( size == 4 ) val = (int)*LONGP(cp, i);
        if ( val < 0 ) val = (-val);
        if ( val > max ) max = val;
    }
{
    signed char *cp;
    int len, size, val = 0;
    int i;
    int min = 0x7fffffff, max = -0x7fffffff;

    if (!PyArg_ParseTuple(args, "s#i:minmax", &cp, &len, &size))
        return NULL;
    if (!audioop_check_parameters(len, size))
        return NULL;
    for (i = 0; i < len; i += size) {
        if (size == 1) val = (int) *CHARP(cp, i);
        else if (size == 2) val = (int) *SHORTP(cp, i);
        else if (size == 4) val = (int) *LONGP(cp, i);
        if (val > max) max = val;
        if (val < min) min = val;
    }
{
    signed char *cp;
    int len, size, val = 0;
    int i;
    double avg = 0.0;

    if ( !PyArg_ParseTuple(args, "s#i:avg", &cp, &len, &size) )
        return 0;
    if (!audioop_check_parameters(len, size))
        return NULL;
    for ( i=0; i<len; i+= size) {
        if ( size == 1 )      val = (int)*CHARP(cp, i);
        else if ( size == 2 ) val = (int)*SHORTP(cp, i);
        else if ( size == 4 ) val = (int)*LONGP(cp, i);
        avg += val;
    }
{
    signed char *cp;
    int len, size, val = 0;
    int i;
    double sum_squares = 0.0;

    if ( !PyArg_ParseTuple(args, "s#i:rms", &cp, &len, &size) )
        return 0;
    if (!audioop_check_parameters(len, size))
        return NULL;
    for ( i=0; i<len; i+= size) {
        if ( size == 1 )      val = (int)*CHARP(cp, i);
        else if ( size == 2 ) val = (int)*SHORTP(cp, i);
        else if ( size == 4 ) val = (int)*LONGP(cp, i);
        sum_squares += (double)val*(double)val;
    }
{
    signed char *cp;
    int len, size, val = 0, prevval = 0, prevextremevalid = 0,
        prevextreme = 0;
    int i;
    double avg = 0.0;
    int diff, prevdiff, extremediff, nextreme = 0;

    if ( !PyArg_ParseTuple(args, "s#i:avgpp", &cp, &len, &size) )
        return 0;
    if (!audioop_check_parameters(len, size))
        return NULL;
    /* Compute first delta value ahead. Also automatically makes us
    ** skip the first extreme value
    */
    if ( size == 1 )      prevval = (int)*CHARP(cp, 0);
    else if ( size == 2 ) prevval = (int)*SHORTP(cp, 0);
    else if ( size == 4 ) prevval = (int)*LONGP(cp, 0);
    if ( size == 1 )      val = (int)*CHARP(cp, size);
    else if ( size == 2 ) val = (int)*SHORTP(cp, size);
    else if ( size == 4 ) val = (int)*LONGP(cp, size);
    prevdiff = val - prevval;

    for ( i=size; i<len; i+= size) {
        if ( size == 1 )      val = (int)*CHARP(cp, i);
        else if ( size == 2 ) val = (int)*SHORTP(cp, i);
        else if ( size == 4 ) val = (int)*LONGP(cp, i);
        diff = val - prevval;
        if ( diff*prevdiff < 0 ) {
            /* Derivative changed sign. Compute difference to last
            ** extreme value and remember.
            */
            if ( prevextremevalid ) {
                extremediff = prevval - prevextreme;
                if ( extremediff < 0 )
                    extremediff = -extremediff;
                avg += extremediff;
                nextreme++;
            }
            prevextremevalid = 1;
            prevextreme = prevval;
        }
        prevval = val;
        if ( diff != 0 )
            prevdiff = diff;
    }
{
    signed char *cp;
    int len, size, val = 0, prevval = 0, prevextremevalid = 0,
        prevextreme = 0;
    int i;
    int max = 0;
    int diff, prevdiff, extremediff;

    if ( !PyArg_ParseTuple(args, "s#i:maxpp", &cp, &len, &size) )
        return 0;
    if (!audioop_check_parameters(len, size))
        return NULL;
    /* Compute first delta value ahead. Also automatically makes us
    ** skip the first extreme value
    */
    if ( size == 1 )      prevval = (int)*CHARP(cp, 0);
    else if ( size == 2 ) prevval = (int)*SHORTP(cp, 0);
    else if ( size == 4 ) prevval = (int)*LONGP(cp, 0);
    if ( size == 1 )      val = (int)*CHARP(cp, size);
    else if ( size == 2 ) val = (int)*SHORTP(cp, size);
    else if ( size == 4 ) val = (int)*LONGP(cp, size);
    prevdiff = val - prevval;

    for ( i=size; i<len; i+= size) {
        if ( size == 1 )      val = (int)*CHARP(cp, i);
        else if ( size == 2 ) val = (int)*SHORTP(cp, i);
        else if ( size == 4 ) val = (int)*LONGP(cp, i);
        diff = val - prevval;
        if ( diff*prevdiff < 0 ) {
            /* Derivative changed sign. Compute difference to
            ** last extreme value and remember.
            */
            if ( prevextremevalid ) {
                extremediff = prevval - prevextreme;
                if ( extremediff < 0 )
                    extremediff = -extremediff;
                if ( extremediff > max )
                    max = extremediff;
            }
            prevextremevalid = 1;
            prevextreme = prevval;
        }
        prevval = val;
        if ( diff != 0 )
            prevdiff = diff;
    }
{
    signed char *cp;
    int len, size, val = 0;
    int i;
    int prevval, ncross;

    if ( !PyArg_ParseTuple(args, "s#i:cross", &cp, &len, &size) )
        return 0;
    if (!audioop_check_parameters(len, size))
        return NULL;
    ncross = -1;
    prevval = 17; /* Anything <> 0,1 */
    for ( i=0; i<len; i+= size) {
        if ( size == 1 )      val = ((int)*CHARP(cp, i)) >> 7;
        else if ( size == 2 ) val = ((int)*SHORTP(cp, i)) >> 15;
        else if ( size == 4 ) val = ((int)*LONGP(cp, i)) >> 31;
        val = val & 1;
        if ( val != prevval ) ncross++;
        prevval = val;
    }
{
    signed char *cp, *ncp;
    int len, size, val = 0;
    double factor, fval, maxval;
    PyObject *rv;
    int i;

    if ( !PyArg_ParseTuple(args, "s#id:mul", &cp, &len, &size, &factor ) )
        return 0;
    if (!audioop_check_parameters(len, size))
        return NULL;

    if ( size == 1 ) maxval = (double) 0x7f;
    else if ( size == 2 ) maxval = (double) 0x7fff;
    else if ( size == 4 ) maxval = (double) 0x7fffffff;
    else {
        PyErr_SetString(AudioopError, "Size should be 1, 2 or 4");
        return 0;
    }
{
    signed char *cp, *ncp;
    int len, size, val1 = 0, val2 = 0;
    double fac1, fac2, fval, maxval;
    PyObject *rv;
    int i;

    if ( !PyArg_ParseTuple(args, "s#idd:tomono",
                           &cp, &len, &size, &fac1, &fac2 ) )
        return 0;
    if (!audioop_check_parameters(len, size))
        return NULL;
    if (((len / size) & 1) != 0) {
        PyErr_SetString(AudioopError, "not a whole number of frames");
        return NULL;
    }
{
    signed char *cp, *ncp;
    int len, size, val1, val2, val = 0;
    double fac1, fac2, fval, maxval;
    PyObject *rv;
    int i;

    if ( !PyArg_ParseTuple(args, "s#idd:tostereo",
                           &cp, &len, &size, &fac1, &fac2 ) )
        return 0;
    if (!audioop_check_parameters(len, size))
        return NULL;

    if ( size == 1 ) maxval = (double) 0x7f;
    else if ( size == 2 ) maxval = (double) 0x7fff;
    else if ( size == 4 ) maxval = (double) 0x7fffffff;
    else {
        PyErr_SetString(AudioopError, "Size should be 1, 2 or 4");
        return 0;
    }
{
    signed char *cp1, *cp2, *ncp;
    int len1, len2, size, val1 = 0, val2 = 0, maxval, newval;
    PyObject *rv;
    int i;

    if ( !PyArg_ParseTuple(args, "s#s#i:add",
                      &cp1, &len1, &cp2, &len2, &size ) )
        return 0;
    if (!audioop_check_parameters(len1, size))
        return NULL;
    if ( len1 != len2 ) {
        PyErr_SetString(AudioopError, "Lengths should be the same");
        return 0;
    }
{
    signed char *cp, *ncp;
    int len, size, val = 0;
    PyObject *rv;
    int i;
    int bias;

    if ( !PyArg_ParseTuple(args, "s#ii:bias",
                      &cp, &len, &size , &bias) )
        return 0;

    if (!audioop_check_parameters(len, size))
        return NULL;

    rv = PyString_FromStringAndSize(NULL, len);
    if ( rv == 0 )
        return 0;
    ncp = (signed char *)PyString_AsString(rv);


    for ( i=0; i < len; i += size ) {
        if ( size == 1 )      val = (int)*CHARP(cp, i);
        else if ( size == 2 ) val = (int)*SHORTP(cp, i);
        else if ( size == 4 ) val = (int)*LONGP(cp, i);

        if ( size == 1 )      *CHARP(ncp, i) = (signed char)(val+bias);
        else if ( size == 2 ) *SHORTP(ncp, i) = (short)(val+bias);
        else if ( size == 4 ) *LONGP(ncp, i) = (Py_Int32)(val+bias);
    }
{
    signed char *cp;
    unsigned char *ncp;
    int len, size, val = 0;
    PyObject *rv;
    int i, j;

    if ( !PyArg_ParseTuple(args, "s#i:reverse",
                      &cp, &len, &size) )
        return 0;

    if (!audioop_check_parameters(len, size))
        return NULL;

    rv = PyString_FromStringAndSize(NULL, len);
    if ( rv == 0 )
        return 0;
    ncp = (unsigned char *)PyString_AsString(rv);

    for ( i=0; i < len; i += size ) {
        if ( size == 1 )      val = ((int)*CHARP(cp, i)) << 8;
        else if ( size == 2 ) val = (int)*SHORTP(cp, i);
        else if ( size == 4 ) val = ((int)*LONGP(cp, i)) >> 16;

        j = len - i - size;

        if ( size == 1 )      *CHARP(ncp, j) = (signed char)(val >> 8);
        else if ( size == 2 ) *SHORTP(ncp, j) = (short)(val);
        else if ( size == 4 ) *LONGP(ncp, j) = (Py_Int32)(val<<16);
    }
{
    signed char *cp;
    unsigned char *ncp;
    int len, size, size2, val = 0;
    PyObject *rv;
    int i, j;

    if ( !PyArg_ParseTuple(args, "s#ii:lin2lin",
                      &cp, &len, &size, &size2) )
        return 0;

    if (!audioop_check_parameters(len, size))
        return NULL;
    if (!audioop_check_size(size2))
        return NULL;

    if (len/size > INT_MAX/size2) {
        PyErr_SetString(PyExc_MemoryError,
                        "not enough memory for output buffer");
        return 0;
    }
{
    char *cp, *ncp;
    int len, size, nchannels, inrate, outrate, weightA, weightB;
    int chan, d, *prev_i, *cur_i, cur_o;
    PyObject *state, *samps, *str, *rv = NULL;
    int bytes_per_frame;

    weightA = 1;
    weightB = 0;
    if (!PyArg_ParseTuple(args, "s#iiiiO|ii:ratecv", &cp, &len, &size,
                          &nchannels, &inrate, &outrate, &state,
                          &weightA, &weightB))
        return NULL;
    if (!audioop_check_size(size))
        return NULL;
    if (nchannels < 1) {
        PyErr_SetString(AudioopError, "# of channels should be >= 1");
        return NULL;
    }
{
    signed char *cp;
    unsigned char *ncp;
    int len, size, val = 0;
    PyObject *rv;
    int i;

    if ( !PyArg_ParseTuple(args, "s#i:lin2ulaw",
                           &cp, &len, &size) )
        return 0 ;

    if (!audioop_check_parameters(len, size))
        return NULL;

    rv = PyString_FromStringAndSize(NULL, len/size);
    if ( rv == 0 )
        return 0;
    ncp = (unsigned char *)PyString_AsString(rv);

    for ( i=0; i < len; i += size ) {
        if ( size == 1 )      val = ((int)*CHARP(cp, i)) << 8;
        else if ( size == 2 ) val = (int)*SHORTP(cp, i);
        else if ( size == 4 ) val = ((int)*LONGP(cp, i)) >> 16;

        *ncp++ = st_14linear2ulaw(val);
    }
{
    unsigned char *cp;
    unsigned char cval;
    signed char *ncp;
    int len, size, val;
    PyObject *rv;
    int i;

    if ( !PyArg_ParseTuple(args, "s#i:ulaw2lin",
                           &cp, &len, &size) )
        return 0;

    if (!audioop_check_parameters(len, size))
        return NULL;

    if (len > INT_MAX/size) {
        PyErr_SetString(PyExc_MemoryError,
                        "not enough memory for output buffer");
        return 0;
    }
{
    signed char *cp;
    unsigned char *ncp;
    int len, size, val = 0;
    PyObject *rv;
    int i;

    if ( !PyArg_ParseTuple(args, "s#i:lin2alaw",
                           &cp, &len, &size) )
        return 0;

    if (!audioop_check_parameters(len, size))
        return NULL;

    rv = PyString_FromStringAndSize(NULL, len/size);
    if ( rv == 0 )
        return 0;
    ncp = (unsigned char *)PyString_AsString(rv);

    for ( i=0; i < len; i += size ) {
        if ( size == 1 )      val = ((int)*CHARP(cp, i)) << 8;
        else if ( size == 2 ) val = (int)*SHORTP(cp, i);
        else if ( size == 4 ) val = ((int)*LONGP(cp, i)) >> 16;

        *ncp++ = st_linear2alaw(val);
    }
{
    unsigned char *cp;
    unsigned char cval;
    signed char *ncp;
    int len, size, val;
    PyObject *rv;
    int i;

    if ( !PyArg_ParseTuple(args, "s#i:alaw2lin",
                           &cp, &len, &size) )
        return 0;

    if (!audioop_check_parameters(len, size))
        return NULL;

    if (len > INT_MAX/size) {
        PyErr_SetString(PyExc_MemoryError,
                        "not enough memory for output buffer");
        return 0;
    }
{
    signed char *cp;
    signed char *ncp;
    int len, size, val = 0, step, valpred, delta,
        index, sign, vpdiff, diff;
    PyObject *rv, *state, *str;
    int i, outputbuffer = 0, bufferstep;

    if ( !PyArg_ParseTuple(args, "s#iO:lin2adpcm",
                           &cp, &len, &size, &state) )
        return 0;

    if (!audioop_check_parameters(len, size))
        return NULL;

    str = PyString_FromStringAndSize(NULL, len/(size*2));
    if ( str == 0 )
        return 0;
    ncp = (signed char *)PyString_AsString(str);

    /* Decode state, should have (value, step) */
    if ( state == Py_None ) {
        /* First time, it seems. Set defaults */
        valpred = 0;
        index = 0;
    } else if ( !PyArg_ParseTuple(state, "ii", &valpred, &index) )
        return 0;

    step = stepsizeTable[index];
    bufferstep = 1;

    for ( i=0; i < len; i += size ) {
        if ( size == 1 )      val = ((int)*CHARP(cp, i)) << 8;
        else if ( size == 2 ) val = (int)*SHORTP(cp, i);
        else if ( size == 4 ) val = ((int)*LONGP(cp, i)) >> 16;

        /* Step 1 - compute difference with previous value */
        diff = val - valpred;
        sign = (diff < 0) ? 8 : 0;
        if ( sign ) diff = (-diff);

        /* Step 2 - Divide and clamp */
        /* Note:
        ** This code *approximately* computes:
        **    delta = diff*4/step;
        **    vpdiff = (delta+0.5)*step/4;
        ** but in shift step bits are dropped. The net result of this
        ** is that even if you have fast mul/div hardware you cannot
        ** put it to good use since the fixup would be too expensive.
        */
        delta = 0;
        vpdiff = (step >> 3);

        if ( diff >= step ) {
            delta = 4;
            diff -= step;
            vpdiff += step;
        }
        step >>= 1;
        if ( diff >= step  ) {
            delta |= 2;
            diff -= step;
            vpdiff += step;
        }
        step >>= 1;
        if ( diff >= step ) {
            delta |= 1;
            vpdiff += step;
        }

        /* Step 3 - Update previous value */
        if ( sign )
            valpred -= vpdiff;
        else
            valpred += vpdiff;

        /* Step 4 - Clamp previous value to 16 bits */
        if ( valpred > 32767 )
            valpred = 32767;
        else if ( valpred < -32768 )
            valpred = -32768;

        /* Step 5 - Assemble value, update index and step values */
        delta |= sign;

        index += indexTable[delta];
        if ( index < 0 ) index = 0;
        if ( index > 88 ) index = 88;
        step = stepsizeTable[index];

        /* Step 6 - Output value */
        if ( bufferstep ) {
            outputbuffer = (delta << 4) & 0xf0;
        } else {
            *ncp++ = (delta & 0x0f) | outputbuffer;
        }
        bufferstep = !bufferstep;
    }
    rv = Py_BuildValue("(O(ii))", str, valpred, index);
    Py_DECREF(str);
    return rv;
}

static PyObject *
audioop_adpcm2lin(PyObject *self, PyObject *args)
{
    signed char *cp;
    signed char *ncp;
    int len, size, valpred, step, delta, index, sign, vpdiff;
    PyObject *rv, *str, *state;
    int i, inputbuffer = 0, bufferstep;

    if ( !PyArg_ParseTuple(args, "s#iO:adpcm2lin",
                           &cp, &len, &size, &state) )
        return 0;

    if (!audioop_check_parameters(len, size))
        return NULL;

    /* Decode state, should have (value, step) */
    if ( state == Py_None ) {
        /* First time, it seems. Set defaults */
        valpred = 0;
        index = 0;
    } else if ( !PyArg_ParseTuple(state, "ii", &valpred, &index) )
        return 0;

    if (len > (INT_MAX/2)/size) {
        PyErr_SetString(PyExc_MemoryError,
                        "not enough memory for output buffer");
        return 0;
    }
    str = PyString_FromStringAndSize(NULL, len*size*2);
    if ( str == 0 )
        return 0;
    ncp = (signed char *)PyString_AsString(str);

    step = stepsizeTable[index];
    bufferstep = 0;

    for ( i=0; i < len*size*2; i += size ) {
        /* Step 1 - get the delta value and compute next index */
        if ( bufferstep ) {
            delta = inputbuffer & 0xf;
        } else {
            inputbuffer = *cp++;
            delta = (inputbuffer >> 4) & 0xf;
        }

        bufferstep = !bufferstep;

        /* Step 2 - Find new index value (for later) */
        index += indexTable[delta];
        if ( index < 0 ) index = 0;
        if ( index > 88 ) index = 88;

        /* Step 3 - Separate sign and magnitude */
        sign = delta & 8;
        delta = delta & 7;

        /* Step 4 - Compute difference and new predicted value */
        /*
        ** Computes 'vpdiff = (delta+0.5)*step/4', but see comment
        ** in adpcm_coder.
        */
        vpdiff = step >> 3;
        if ( delta & 4 ) vpdiff += step;
        if ( delta & 2 ) vpdiff += step>>1;
        if ( delta & 1 ) vpdiff += step>>2;

        if ( sign )
            valpred -= vpdiff;
        else
            valpred += vpdiff;

        /* Step 5 - clamp output value */
        if ( valpred > 32767 )
            valpred = 32767;
        else if ( valpred < -32768 )
            valpred = -32768;

        /* Step 6 - Update step value */
        step = stepsizeTable[index];

        /* Step 6 - Output value */
        if ( size == 1 ) *CHARP(ncp, i) = (signed char)(valpred >> 8);
        else if ( size == 2 ) *SHORTP(ncp, i) = (short)(valpred);
        else if ( size == 4 ) *LONGP(ncp, i) = (Py_Int32)(valpred<<16);
    }

    rv = Py_BuildValue("(O(ii))", str, valpred, index);
    Py_DECREF(str);
    return rv;
}

static PyMethodDef audioop_methods[] = {
    { "max", audioop_max, METH_VARARGS },
    { "minmax", audioop_minmax, METH_VARARGS },
    { "avg", audioop_avg, METH_VARARGS },
    { "maxpp", audioop_maxpp, METH_VARARGS },
    { "avgpp", audioop_avgpp, METH_VARARGS },
    { "rms", audioop_rms, METH_VARARGS },
    { "findfit", audioop_findfit, METH_VARARGS },
    { "findmax", audioop_findmax, METH_VARARGS },
    { "findfactor", audioop_findfactor, METH_VARARGS },
    { "cross", audioop_cross, METH_VARARGS },
    { "mul", audioop_mul, METH_VARARGS },
    { "add", audioop_add, METH_VARARGS },
    { "bias", audioop_bias, METH_VARARGS },
    { "ulaw2lin", audioop_ulaw2lin, METH_VARARGS },
    { "lin2ulaw", audioop_lin2ulaw, METH_VARARGS },
    { "alaw2lin", audioop_alaw2lin, METH_VARARGS },
    { "lin2alaw", audioop_lin2alaw, METH_VARARGS },
    { "lin2lin", audioop_lin2lin, METH_VARARGS },
    { "adpcm2lin", audioop_adpcm2lin, METH_VARARGS },
    { "lin2adpcm", audioop_lin2adpcm, METH_VARARGS },
    { "tomono", audioop_tomono, METH_VARARGS },
    { "tostereo", audioop_tostereo, METH_VARARGS },
    { "getsample", audioop_getsample, METH_VARARGS },
    { "reverse", audioop_reverse, METH_VARARGS },
    { "ratecv", audioop_ratecv, METH_VARARGS },
    { 0,          0 }
};

PyMODINIT_FUNC
initaudioop(void)
{
    PyObject *m, *d;
    m = Py_InitModule("audioop", audioop_methods);
    if (m == NULL)
        return;
    d = PyModule_GetDict(m);
    if (d == NULL)
        return;
    AudioopError = PyErr_NewException("audioop.error", NULL, NULL);
    if (AudioopError != NULL)
         PyDict_SetItemString(d,"error",AudioopError);
}
{
    signed char *cp;
    signed char *ncp;
    int len, size, valpred, step, delta, index, sign, vpdiff;
    PyObject *rv, *str, *state;
    int i, inputbuffer = 0, bufferstep;

    if ( !PyArg_ParseTuple(args, "s#iO:adpcm2lin",
                           &cp, &len, &size, &state) )
        return 0;

    if (!audioop_check_parameters(len, size))
        return NULL;

    /* Decode state, should have (value, step) */
    if ( state == Py_None ) {
        /* First time, it seems. Set defaults */
        valpred = 0;
        index = 0;
    } else if ( !PyArg_ParseTuple(state, "ii", &valpred, &index) )
        return 0;

    if (len > (INT_MAX/2)/size) {
        PyErr_SetString(PyExc_MemoryError,
                        "not enough memory for output buffer");
        return 0;
    }
    str = PyString_FromStringAndSize(NULL, len*size*2);
    if ( str == 0 )
        return 0;
    ncp = (signed char *)PyString_AsString(str);

    step = stepsizeTable[index];
    bufferstep = 0;

    for ( i=0; i < len*size*2; i += size ) {
        /* Step 1 - get the delta value and compute next index */
        if ( bufferstep ) {
            delta = inputbuffer & 0xf;
        } else {
            inputbuffer = *cp++;
            delta = (inputbuffer >> 4) & 0xf;
        }

        bufferstep = !bufferstep;

        /* Step 2 - Find new index value (for later) */
        index += indexTable[delta];
        if ( index < 0 ) index = 0;
        if ( index > 88 ) index = 88;

        /* Step 3 - Separate sign and magnitude */
        sign = delta & 8;
        delta = delta & 7;

        /* Step 4 - Compute difference and new predicted value */
        /*
        ** Computes 'vpdiff = (delta+0.5)*step/4', but see comment
        ** in adpcm_coder.
        */
        vpdiff = step >> 3;
        if ( delta & 4 ) vpdiff += step;
        if ( delta & 2 ) vpdiff += step>>1;
        if ( delta & 1 ) vpdiff += step>>2;

        if ( sign )
            valpred -= vpdiff;
        else
            valpred += vpdiff;

        /* Step 5 - clamp output value */
        if ( valpred > 32767 )
            valpred = 32767;
        else if ( valpred < -32768 )
            valpred = -32768;

        /* Step 6 - Update step value */
        step = stepsizeTable[index];

        /* Step 6 - Output value */
        if ( size == 1 ) *CHARP(ncp, i) = (signed char)(valpred >> 8);
        else if ( size == 2 ) *SHORTP(ncp, i) = (short)(valpred);
        else if ( size == 4 ) *LONGP(ncp, i) = (Py_Int32)(valpred<<16);
    }

    rv = Py_BuildValue("(O(ii))", str, valpred, index);
    Py_DECREF(str);
    return rv;
}

static PyMethodDef audioop_methods[] = {
    { "max", audioop_max, METH_VARARGS },
    { "minmax", audioop_minmax, METH_VARARGS },
    { "avg", audioop_avg, METH_VARARGS },
    { "maxpp", audioop_maxpp, METH_VARARGS },
    { "avgpp", audioop_avgpp, METH_VARARGS },
    { "rms", audioop_rms, METH_VARARGS },
    { "findfit", audioop_findfit, METH_VARARGS },
    { "findmax", audioop_findmax, METH_VARARGS },
    { "findfactor", audioop_findfactor, METH_VARARGS },
    { "cross", audioop_cross, METH_VARARGS },
    { "mul", audioop_mul, METH_VARARGS },
    { "add", audioop_add, METH_VARARGS },
    { "bias", audioop_bias, METH_VARARGS },
    { "ulaw2lin", audioop_ulaw2lin, METH_VARARGS },
    { "lin2ulaw", audioop_lin2ulaw, METH_VARARGS },
    { "alaw2lin", audioop_alaw2lin, METH_VARARGS },
    { "lin2alaw", audioop_lin2alaw, METH_VARARGS },
    { "lin2lin", audioop_lin2lin, METH_VARARGS },
    { "adpcm2lin", audioop_adpcm2lin, METH_VARARGS },
    { "lin2adpcm", audioop_lin2adpcm, METH_VARARGS },
    { "tomono", audioop_tomono, METH_VARARGS },
    { "tostereo", audioop_tostereo, METH_VARARGS },
    { "getsample", audioop_getsample, METH_VARARGS },
    { "reverse", audioop_reverse, METH_VARARGS },
    { "ratecv", audioop_ratecv, METH_VARARGS },
    { 0,          0 }
};

PyMODINIT_FUNC
initaudioop(void)
{
    PyObject *m, *d;
    m = Py_InitModule("audioop", audioop_methods);
    if (m == NULL)
        return;
    d = PyModule_GetDict(m);
    if (d == NULL)
        return;
    AudioopError = PyErr_NewException("audioop.error", NULL, NULL);
    if (AudioopError != NULL)
         PyDict_SetItemString(d,"error",AudioopError);
}