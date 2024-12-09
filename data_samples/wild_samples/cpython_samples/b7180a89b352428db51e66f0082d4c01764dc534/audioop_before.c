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

static PyObject *
audioop_getsample(PyObject *self, PyObject *args)
{
        signed char *cp;
        int len, size, val = 0;
        int i;

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
        if ( size == 1 )      val = (int)*CHARP(cp, i);
        else if ( size == 2 ) val = (int)*SHORTP(cp, i*2);
        else if ( size == 4 ) val = (int)*LONGP(cp, i*4);
        return PyInt_FromLong(val);
}
{
        signed char *cp;
        int len, size, val = 0;
        int i;

        if ( !PyArg_ParseTuple(args, "s#ii:getsample", &cp, &len, &size, &i) )
                return 0;
        if ( size != 1 && size != 2 && size != 4 ) {
                PyErr_SetString(AudioopError, "Size should be 1, 2 or 4");
                return 0;
        }
{
        signed char *cp;
        int len, size, val = 0;
        int i;
        int max = 0;

        if ( !PyArg_ParseTuple(args, "s#i:max", &cp, &len, &size) )
                return 0;
        if ( size != 1 && size != 2 && size != 4 ) {
                PyErr_SetString(AudioopError, "Size should be 1, 2 or 4");
                return 0;
        }
{
        signed char *cp;
        int len, size, val = 0;
        int i;
        int min = 0x7fffffff, max = -0x7fffffff;

        if (!PyArg_ParseTuple(args, "s#i:minmax", &cp, &len, &size))
                return NULL;
        if (size != 1 && size != 2 && size != 4) {
                PyErr_SetString(AudioopError, "Size should be 1, 2 or 4");
                return NULL;
        }
{
        signed char *cp;
        int len, size, val = 0;
        int i;
        double avg = 0.0;

        if ( !PyArg_ParseTuple(args, "s#i:avg", &cp, &len, &size) )
                return 0;
        if ( size != 1 && size != 2 && size != 4 ) {
                PyErr_SetString(AudioopError, "Size should be 1, 2 or 4");
                return 0;
        }
{
        signed char *cp;
        int len, size, val = 0;
        int i;
        double sum_squares = 0.0;

        if ( !PyArg_ParseTuple(args, "s#i:rms", &cp, &len, &size) )
                return 0;
        if ( size != 1 && size != 2 && size != 4 ) {
                PyErr_SetString(AudioopError, "Size should be 1, 2 or 4");
                return 0;
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
        if ( size != 1 && size != 2 && size != 4 ) {
                PyErr_SetString(AudioopError, "Size should be 1, 2 or 4");
                return 0;
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
        if ( size != 1 && size != 2 && size != 4 ) {
                PyErr_SetString(AudioopError, "Size should be 1, 2 or 4");
                return 0;
        }
{
        signed char *cp;
        int len, size, val = 0;
        int i;
        int prevval, ncross;

        if ( !PyArg_ParseTuple(args, "s#i:cross", &cp, &len, &size) )
                return 0;
        if ( size != 1 && size != 2 && size != 4 ) {
                PyErr_SetString(AudioopError, "Size should be 1, 2 or 4");
                return 0;
        }
{
        signed char *cp, *ncp;
        int len, size, val = 0;
        double factor, fval, maxval;
        PyObject *rv;
        int i;

        if ( !PyArg_ParseTuple(args, "s#id:mul", &cp, &len, &size, &factor ) )
                return 0;
    
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
    
        if ( size == 1 ) maxval = (double) 0x7f;
        else if ( size == 2 ) maxval = (double) 0x7fff;
        else if ( size == 4 ) maxval = (double) 0x7fffffff;
        else {
                PyErr_SetString(AudioopError, "Size should be 1, 2 or 4");
                return 0;
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

        if ( size != 1 && size != 2 && size != 4) {
                PyErr_SetString(AudioopError, "Size should be 1, 2 or 4");
                return 0;
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

        if ( size != 1 && size != 2 && size != 4 ) {
                PyErr_SetString(AudioopError, "Size should be 1, 2 or 4");
                return 0;
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

        if ( (size != 1 && size != 2 && size != 4) ||
             (size2 != 1 && size2 != 2 && size2 != 4)) {
                PyErr_SetString(AudioopError, "Size should be 1, 2 or 4");
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
        if (size != 1 && size != 2 && size != 4) {
                PyErr_SetString(AudioopError, "Size should be 1, 2 or 4");
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

        if ( size != 1 && size != 2 && size != 4) {
                PyErr_SetString(AudioopError, "Size should be 1, 2 or 4");
                return 0;
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

        if ( size != 1 && size != 2 && size != 4) {
                PyErr_SetString(AudioopError, "Size should be 1, 2 or 4");
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

        if ( size != 1 && size != 2 && size != 4) {
                PyErr_SetString(AudioopError, "Size should be 1, 2 or 4");
                return 0;
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

        if ( size != 1 && size != 2 && size != 4) {
                PyErr_SetString(AudioopError, "Size should be 1, 2 or 4");
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
    

        if ( size != 1 && size != 2 && size != 4) {
                PyErr_SetString(AudioopError, "Size should be 1, 2 or 4");
                return 0;
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

        if ( size != 1 && size != 2 && size != 4) {
                PyErr_SetString(AudioopError, "Size should be 1, 2 or 4");
                return 0;
        }