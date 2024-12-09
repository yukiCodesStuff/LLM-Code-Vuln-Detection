{
    assert(IS_MEDIUM_VALUE(x));
    return ((stwodigits)Py_SIZE(x)) * x->ob_digit[0];
}

#define IS_SMALL_INT(ival) (-_PY_NSMALLNEGINTS <= (ival) && (ival) < _PY_NSMALLPOSINTS)
#define IS_SMALL_UINT(ival) ((ival) < _PY_NSMALLPOSINTS)

static inline void
_Py_DECREF_INT(PyLongObject *op)
{
    assert(PyLong_CheckExact(op));
    _Py_DECREF_SPECIALIZED((PyObject *)op, (destructor)PyObject_Free);
}
    while (rem >= tenpow) {
        tenpow *= 10;
        strlen++;
    }
    if (writer) {
        if (_PyUnicodeWriter_Prepare(writer, strlen, '9') == -1) {
            Py_DECREF(scratch);
            return -1;
        }
    if (str[0] == '_') {
        /* May not start with underscores. */
        goto onError;
    }

    start = str;
    if ((base & (base - 1)) == 0) {
        int res = long_from_binary_base(&str, base, &z);
        if (res < 0) {
            /* Syntax error. */
            goto onError;
        }
    }
        if (prev == '_') {
            /* Trailing underscore not allowed. */
            /* Set error pointer to first underscore. */
            str = lastdigit + 1;
            goto onError;
        }

        /* Create an int object that can contain the largest possible
         * integer with this base and length.  Note that there's no
         * need to initialize z->ob_digit -- no slot is read up before
         * being stored into.
         */
        double fsize_z = (double)digits * log_base_BASE[base] + 1.0;
        if (fsize_z > (double)MAX_LONG_DIGITS) {
            /* The same exception as in _PyLong_New(). */
            PyErr_SetString(PyExc_OverflowError,
                            "too many digits in integer");
            return NULL;
        }
        size_z = (Py_ssize_t)fsize_z;
        /* Uncomment next line to test exceedingly rare copy code */
        /* size_z = 1; */
        assert(size_z > 0);
        z = _PyLong_New(size_z);
        if (z == NULL) {
            return NULL;
        }
        Py_SET_SIZE(z, 0);

        /* `convwidth` consecutive input digits are treated as a single
         * digit in base `convmultmax`.
         */
        convwidth = convwidth_base[base];
        convmultmax = convmultmax_base[base];

        /* Work ;-) */
        while (str < scan) {
            if (*str == '_') {
                str++;
                continue;
            }
        if (obase != NULL) {
            PyErr_SetString(PyExc_TypeError,
                            "int() missing string argument");
            return NULL;
        }
        return PyLong_FromLong(0L);
    }
    if (obase == NULL)
        return PyNumber_Long(x);

    base = PyNumber_AsSsize_t(obase, NULL);
    if (base == -1 && PyErr_Occurred())
        return NULL;
    if ((base != 0 && base < 2) || base > 36) {
        PyErr_SetString(PyExc_ValueError,
                        "int() base must be >= 2 and <= 36, or 0");
        return NULL;
    }

    if (PyUnicode_Check(x))
        return PyLong_FromUnicodeObject(x, (int)base);
    else if (PyByteArray_Check(x) || PyBytes_Check(x)) {
        const char *string;
        if (PyByteArray_Check(x))
            string = PyByteArray_AS_STRING(x);
        else
            string = PyBytes_AS_STRING(x);
        return _PyLong_FromBytes(string, Py_SIZE(x), (int)base);
    }
    else {
        PyErr_SetString(PyExc_TypeError,
                        "int() can't convert non-string with explicit base");
        return NULL;
    }
}

/* Wimpy, slow approach to tp_new calls for subtypes of int:
   first create a regular int from whatever arguments we got,
   then allocate a subtype instance and initialize it from
   the regular int.  The regular int is then thrown away.
*/
static PyObject *
long_subtype_new(PyTypeObject *type, PyObject *x, PyObject *obase)
{
    PyLongObject *tmp, *newobj;
    Py_ssize_t i, n;

    assert(PyType_IsSubtype(type, &PyLong_Type));
    tmp = (PyLongObject *)long_new_impl(&PyLong_Type, x, obase);
    if (tmp == NULL)
        return NULL;
    assert(PyLong_Check(tmp));
    n = Py_SIZE(tmp);
    if (n < 0)
        n = -n;
    newobj = (PyLongObject *)type->tp_alloc(type, n);
    if (newobj == NULL) {
        Py_DECREF(tmp);
        return NULL;
    }
    assert(PyLong_Check(newobj));
    Py_SET_SIZE(newobj, Py_SIZE(tmp));
    for (i = 0; i < n; i++) {
        newobj->ob_digit[i] = tmp->ob_digit[i];
    }
    Py_DECREF(tmp);
    return (PyObject *)newobj;
}

/*[clinic input]
int.__getnewargs__
[clinic start generated code]*/

static PyObject *
int___getnewargs___impl(PyObject *self)
/*[clinic end generated code: output=839a49de3f00b61b input=5904770ab1fb8c75]*/
{
    return Py_BuildValue("(N)", _PyLong_Copy((PyLongObject *)self));
}

static PyObject *
long_get0(PyObject *Py_UNUSED(self), void *Py_UNUSED(context))
{
    return PyLong_FromLong(0L);
}

static PyObject *
long_get1(PyObject *Py_UNUSED(self), void *Py_UNUSED(ignored))
{
    return PyLong_FromLong(1L);
}

/*[clinic input]
int.__format__

    format_spec: unicode
    /
[clinic start generated code]*/

static PyObject *
int___format___impl(PyObject *self, PyObject *format_spec)
/*[clinic end generated code: output=b4929dee9ae18689 input=e31944a9b3e428b7]*/
{
    _PyUnicodeWriter writer;
    int ret;

    _PyUnicodeWriter_Init(&writer);
    ret = _PyLong_FormatAdvancedWriter(
        &writer,
        self,
        format_spec, 0, PyUnicode_GET_LENGTH(format_spec));
    if (ret == -1) {
        _PyUnicodeWriter_Dealloc(&writer);
        return NULL;
    }
    return _PyUnicodeWriter_Finish(&writer);
}

/* Return a pair (q, r) such that a = b * q + r, and
   abs(r) <= abs(b)/2, with equality possible only if q is even.
   In other words, q == a / b, rounded to the nearest integer using
   round-half-to-even. */

PyObject *
_PyLong_DivmodNear(PyObject *a, PyObject *b)
{
    PyLongObject *quo = NULL, *rem = NULL;
    PyObject *twice_rem, *result, *temp;
    int quo_is_odd, quo_is_neg;
    Py_ssize_t cmp;

    /* Equivalent Python code:

       def divmod_near(a, b):
           q, r = divmod(a, b)
           # round up if either r / b > 0.5, or r / b == 0.5 and q is odd.
           # The expression r / b > 0.5 is equivalent to 2 * r > b if b is
           # positive, 2 * r < b if b negative.
           greater_than_half = 2*r > b if b > 0 else 2*r < b
           exactly_half = 2*r == b
           if greater_than_half or exactly_half and q % 2 == 1:
               q += 1
               r -= b
           return q, r

    */
    if (!PyLong_Check(a) || !PyLong_Check(b)) {
        PyErr_SetString(PyExc_TypeError,
                        "non-integer arguments in division");
        return NULL;
    }

    /* Do a and b have different signs?  If so, quotient is negative. */
    quo_is_neg = (Py_SIZE(a) < 0) != (Py_SIZE(b) < 0);

    if (long_divrem((PyLongObject*)a, (PyLongObject*)b, &quo, &rem) < 0)
        goto error;

    /* compare twice the remainder with the divisor, to see
       if we need to adjust the quotient and remainder */
    PyObject *one = _PyLong_GetOne();  // borrowed reference
    twice_rem = long_lshift((PyObject *)rem, one);
    if (twice_rem == NULL)
        goto error;
    if (quo_is_neg) {
        temp = long_neg((PyLongObject*)twice_rem);
        Py_DECREF(twice_rem);
        twice_rem = temp;
        if (twice_rem == NULL)
            goto error;
    }
    cmp = long_compare((PyLongObject *)twice_rem, (PyLongObject *)b);
    Py_DECREF(twice_rem);

    quo_is_odd = Py_SIZE(quo) != 0 && ((quo->ob_digit[0] & 1) != 0);
    if ((Py_SIZE(b) < 0 ? cmp < 0 : cmp > 0) || (cmp == 0 && quo_is_odd)) {
        /* fix up quotient */
        if (quo_is_neg)
            temp = long_sub(quo, (PyLongObject *)one);
        else
            temp = long_add(quo, (PyLongObject *)one);
        Py_DECREF(quo);
        quo = (PyLongObject *)temp;
        if (quo == NULL)
            goto error;
        /* and remainder */
        if (quo_is_neg)
            temp = long_add(rem, (PyLongObject *)b);
        else
            temp = long_sub(rem, (PyLongObject *)b);
        Py_DECREF(rem);
        rem = (PyLongObject *)temp;
        if (rem == NULL)
            goto error;
    }

    result = PyTuple_New(2);
    if (result == NULL)
        goto error;

    /* PyTuple_SET_ITEM steals references */
    PyTuple_SET_ITEM(result, 0, (PyObject *)quo);
    PyTuple_SET_ITEM(result, 1, (PyObject *)rem);
    return result;

  error:
    Py_XDECREF(quo);
    Py_XDECREF(rem);
    return NULL;
}

/*[clinic input]
int.__round__

    ndigits as o_ndigits: object = NULL
    /

Rounding an Integral returns itself.

Rounding with an ndigits argument also returns an integer.
[clinic start generated code]*/

static PyObject *
int___round___impl(PyObject *self, PyObject *o_ndigits)
/*[clinic end generated code: output=954fda6b18875998 input=1614cf23ec9e18c3]*/
{
    PyObject *temp, *result, *ndigits;

    /* To round an integer m to the nearest 10**n (n positive), we make use of
     * the divmod_near operation, defined by:
     *
     *   divmod_near(a, b) = (q, r)
     *
     * where q is the nearest integer to the quotient a / b (the
     * nearest even integer in the case of a tie) and r == a - q * b.
     * Hence q * b = a - r is the nearest multiple of b to a,
     * preferring even multiples in the case of a tie.
     *
     * So the nearest multiple of 10**n to m is:
     *
     *   m - divmod_near(m, 10**n)[1].
     */
    if (o_ndigits == NULL)
        return long_long(self);

    ndigits = _PyNumber_Index(o_ndigits);
    if (ndigits == NULL)
        return NULL;

    /* if ndigits >= 0 then no rounding is necessary; return self unchanged */
    if (Py_SIZE(ndigits) >= 0) {
        Py_DECREF(ndigits);
        return long_long(self);
    }

    /* result = self - divmod_near(self, 10 ** -ndigits)[1] */
    temp = long_neg((PyLongObject*)ndigits);
    Py_DECREF(ndigits);
    ndigits = temp;
    if (ndigits == NULL)
        return NULL;

    result = PyLong_FromLong(10L);
    if (result == NULL) {
        Py_DECREF(ndigits);
        return NULL;
    }

    temp = long_pow(result, ndigits, Py_None);
    Py_DECREF(ndigits);
    Py_DECREF(result);
    result = temp;
    if (result == NULL)
        return NULL;

    temp = _PyLong_DivmodNear(self, result);
    Py_DECREF(result);
    result = temp;
    if (result == NULL)
        return NULL;

    temp = long_sub((PyLongObject *)self,
                    (PyLongObject *)PyTuple_GET_ITEM(result, 1));
    Py_DECREF(result);
    result = temp;

    return result;
}

/*[clinic input]
int.__sizeof__ -> Py_ssize_t

Returns size in memory, in bytes.
[clinic start generated code]*/

static Py_ssize_t
int___sizeof___impl(PyObject *self)
/*[clinic end generated code: output=3303f008eaa6a0a5 input=9b51620c76fc4507]*/
{
    Py_ssize_t res;

    res = offsetof(PyLongObject, ob_digit) + Py_ABS(Py_SIZE(self))*sizeof(digit);
    return res;
}

/*[clinic input]
int.bit_length

Number of bits necessary to represent self in binary.

>>> bin(37)
'0b100101'
>>> (37).bit_length()
6
[clinic start generated code]*/

static PyObject *
int_bit_length_impl(PyObject *self)
/*[clinic end generated code: output=fc1977c9353d6a59 input=e4eb7a587e849a32]*/
{
    PyLongObject *result, *x, *y;
    Py_ssize_t ndigits;
    int msd_bits;
    digit msd;

    assert(self != NULL);
    assert(PyLong_Check(self));

    ndigits = Py_ABS(Py_SIZE(self));
    if (ndigits == 0)
        return PyLong_FromLong(0);

    msd = ((PyLongObject *)self)->ob_digit[ndigits-1];
    msd_bits = bit_length_digit(msd);

    if (ndigits <= PY_SSIZE_T_MAX/PyLong_SHIFT)
        return PyLong_FromSsize_t((ndigits-1)*PyLong_SHIFT + msd_bits);

    /* expression above may overflow; use Python integers instead */
    result = (PyLongObject *)PyLong_FromSsize_t(ndigits - 1);
    if (result == NULL)
        return NULL;
    x = (PyLongObject *)PyLong_FromLong(PyLong_SHIFT);
    if (x == NULL)
        goto error;
    y = (PyLongObject *)long_mul(result, x);
    Py_DECREF(x);
    if (y == NULL)
        goto error;
    Py_DECREF(result);
    result = y;

    x = (PyLongObject *)PyLong_FromLong((long)msd_bits);
    if (x == NULL)
        goto error;
    y = (PyLongObject *)long_add(result, x);
    Py_DECREF(x);
    if (y == NULL)
        goto error;
    Py_DECREF(result);
    result = y;

    return (PyObject *)result;

  error:
    Py_DECREF(result);
    return NULL;
}

static int
popcount_digit(digit d)
{
    // digit can be larger than uint32_t, but only PyLong_SHIFT bits
    // of it will be ever used.
    static_assert(PyLong_SHIFT <= 32, "digit is larger than uint32_t");
    return _Py_popcount32((uint32_t)d);
}

/*[clinic input]
int.bit_count

Number of ones in the binary representation of the absolute value of self.

Also known as the population count.

>>> bin(13)
'0b1101'
>>> (13).bit_count()
3
[clinic start generated code]*/

static PyObject *
int_bit_count_impl(PyObject *self)
/*[clinic end generated code: output=2e571970daf1e5c3 input=7e0adef8e8ccdf2e]*/
{
    assert(self != NULL);
    assert(PyLong_Check(self));

    PyLongObject *z = (PyLongObject *)self;
    Py_ssize_t ndigits = Py_ABS(Py_SIZE(z));
    Py_ssize_t bit_count = 0;

    /* Each digit has up to PyLong_SHIFT ones, so the accumulated bit count
       from the first PY_SSIZE_T_MAX/PyLong_SHIFT digits can't overflow a
       Py_ssize_t. */
    Py_ssize_t ndigits_fast = Py_MIN(ndigits, PY_SSIZE_T_MAX/PyLong_SHIFT);
    for (Py_ssize_t i = 0; i < ndigits_fast; i++) {
        bit_count += popcount_digit(z->ob_digit[i]);
    }

    PyObject *result = PyLong_FromSsize_t(bit_count);
    if (result == NULL) {
        return NULL;
    }

    /* Use Python integers if bit_count would overflow. */
    for (Py_ssize_t i = ndigits_fast; i < ndigits; i++) {
        PyObject *x = PyLong_FromLong(popcount_digit(z->ob_digit[i]));
        if (x == NULL) {
            goto error;
        }
static PyStructSequence_Field int_info_fields[] = {
    {"bits_per_digit", "size of a digit in bits"},
    {"sizeof_digit", "size in bytes of the C type used to represent a digit"},
    {NULL, NULL}
};

static PyStructSequence_Desc int_info_desc = {
    "sys.int_info",   /* name */
    int_info__doc__,  /* doc */
    int_info_fields,  /* fields */
    2                 /* number of fields */
};

PyObject *
PyLong_GetInfo(void)
{
    PyObject* int_info;
    int field = 0;
    int_info = PyStructSequence_New(&Int_InfoType);
    if (int_info == NULL)
        return NULL;
    PyStructSequence_SET_ITEM(int_info, field++,
                              PyLong_FromLong(PyLong_SHIFT));
    PyStructSequence_SET_ITEM(int_info, field++,
                              PyLong_FromLong(sizeof(digit)));
    if (PyErr_Occurred()) {
        Py_CLEAR(int_info);
        return NULL;
    }
    return int_info;
}
{
    PyObject* int_info;
    int field = 0;
    int_info = PyStructSequence_New(&Int_InfoType);
    if (int_info == NULL)
        return NULL;
    PyStructSequence_SET_ITEM(int_info, field++,
                              PyLong_FromLong(PyLong_SHIFT));
    PyStructSequence_SET_ITEM(int_info, field++,
                              PyLong_FromLong(sizeof(digit)));
    if (PyErr_Occurred()) {
        Py_CLEAR(int_info);
        return NULL;
    }
        if (_PyStructSequence_InitBuiltin(&Int_InfoType, &int_info_desc) < 0) {
            return _PyStatus_ERR("can't init int info type");
        }
    }

    return _PyStatus_OK();
}


void
_PyLong_FiniTypes(PyInterpreterState *interp)
{
    if (!_Py_IsMainInterpreter(interp)) {
        return;
    }

    _PyStructSequence_FiniType(&Int_InfoType);
}