{
    const char *starts = s;
    Py_ssize_t startinpos;
    Py_ssize_t endinpos;
    Py_ssize_t outpos;
    PyUnicodeObject *unicode;
    Py_UNICODE *p;
    const unsigned char *q, *e, *aligned_end;
    int bo = 0;       /* assume native ordering by default */
    int native_ordering = 0;
    const char *errmsg = "";
    /* Offsets from q for retrieving byte pairs in the right order. */
#ifdef BYTEORDER_IS_LITTLE_ENDIAN
    int ihi = 1, ilo = 0;
#else
    int ihi = 0, ilo = 1;
#endif
    PyObject *errorHandler = NULL;
    PyObject *exc = NULL;

    /* Note: size will always be longer than the resulting Unicode
       character count */
    unicode = _PyUnicode_New(size);
    if (!unicode)
        return NULL;
    if (size == 0)
        return (PyObject *)unicode;

    /* Unpack UTF-16 encoded data */
    p = unicode->str;
    q = (unsigned char *)s;
    e = q + size - 1;

    if (byteorder)
        bo = *byteorder;

    /* Check for BOM marks (U+FEFF) in the input and adjust current
       byte order setting accordingly. In native mode, the leading BOM
       mark is skipped, in all other modes, it is copied to the output
       stream as-is (giving a ZWNBSP character). */
    if (bo == 0) {
        if (size >= 2) {
            const Py_UNICODE bom = (q[ihi] << 8) | q[ilo];
#ifdef BYTEORDER_IS_LITTLE_ENDIAN
            if (bom == 0xFEFF) {
                q += 2;
                bo = -1;
            }
            else if (bom == 0xFFFE) {
                q += 2;
                bo = 1;
            }
#else
            if (bom == 0xFEFF) {
                q += 2;
                bo = 1;
            }
            else if (bom == 0xFFFE) {
                q += 2;
                bo = -1;
            }
#endif
        }
    }
    else if (bo == 1) {
        /* force BE */
        ihi = 0;
        ilo = 1;
    }
#ifdef BYTEORDER_IS_LITTLE_ENDIAN
    native_ordering = ilo < ihi;
#else
    native_ordering = ilo > ihi;
#endif

    aligned_end = (const unsigned char *) ((size_t) e & ~LONG_PTR_MASK);
    while (q < e) {
        Py_UNICODE ch;
        /* First check for possible aligned read of a C 'long'. Unaligned
           reads are more expensive, better to defer to another iteration. */
        if (!((size_t) q & LONG_PTR_MASK)) {
            /* Fast path for runs of non-surrogate chars. */
            register const unsigned char *_q = q;
            Py_UNICODE *_p = p;
            if (native_ordering) {
                /* Native ordering is simple: as long as the input cannot
                   possibly contain a surrogate char, do an unrolled copy
                   of several 16-bit code points to the target object.
                   The non-surrogate check is done on several input bytes
                   at a time (as many as a C 'long' can contain). */
                while (_q < aligned_end) {
                    unsigned long data = * (unsigned long *) _q;
                    if (data & FAST_CHAR_MASK)
                        break;
                    _p[0] = ((unsigned short *) _q)[0];
                    _p[1] = ((unsigned short *) _q)[1];
#if (SIZEOF_LONG == 8)
                    _p[2] = ((unsigned short *) _q)[2];
                    _p[3] = ((unsigned short *) _q)[3];
#endif
                    _q += SIZEOF_LONG;
                    _p += SIZEOF_LONG / 2;
                }
            }
            else {
                /* Byteswapped ordering is similar, but we must decompose
                   the copy bytewise, and take care of zero'ing out the
                   upper bytes if the target object is in 32-bit units
                   (that is, in UCS-4 builds). */
                while (_q < aligned_end) {
                    unsigned long data = * (unsigned long *) _q;
                    if (data & SWAPPED_FAST_CHAR_MASK)
                        break;
                    /* Zero upper bytes in UCS-4 builds */
#if (Py_UNICODE_SIZE > 2)
                    _p[0] = 0;
                    _p[1] = 0;
#if (SIZEOF_LONG == 8)
                    _p[2] = 0;
                    _p[3] = 0;
#endif
#endif
                    /* Issue #4916; UCS-4 builds on big endian machines must
                       fill the two last bytes of each 4-byte unit. */
#if (!defined(BYTEORDER_IS_LITTLE_ENDIAN) && Py_UNICODE_SIZE > 2)
# define OFF 2
#else
# define OFF 0
#endif
                    ((unsigned char *) _p)[OFF + 1] = _q[0];
                    ((unsigned char *) _p)[OFF + 0] = _q[1];
                    ((unsigned char *) _p)[OFF + 1 + Py_UNICODE_SIZE] = _q[2];
                    ((unsigned char *) _p)[OFF + 0 + Py_UNICODE_SIZE] = _q[3];
#if (SIZEOF_LONG == 8)
                    ((unsigned char *) _p)[OFF + 1 + 2 * Py_UNICODE_SIZE] = _q[4];
                    ((unsigned char *) _p)[OFF + 0 + 2 * Py_UNICODE_SIZE] = _q[5];
                    ((unsigned char *) _p)[OFF + 1 + 3 * Py_UNICODE_SIZE] = _q[6];
                    ((unsigned char *) _p)[OFF + 0 + 3 * Py_UNICODE_SIZE] = _q[7];
#endif
#undef OFF
                    _q += SIZEOF_LONG;
                    _p += SIZEOF_LONG / 2;
                }
            }
            p = _p;
            q = _q;
            if (q >= e)
                break;
        }
        ch = (q[ihi] << 8) | q[ilo];

        q += 2;

        if (ch < 0xD800 || ch > 0xDFFF) {
            *p++ = ch;
            continue;
        }

        /* UTF-16 code pair: */
        if (q > e) {
            errmsg = "unexpected end of data";
            startinpos = (((const char *)q) - 2) - starts;
            endinpos = ((const char *)e) + 1 - starts;
            goto utf16Error;
        }
        if (0xD800 <= ch && ch <= 0xDBFF) {
            Py_UNICODE ch2 = (q[ihi] << 8) | q[ilo];
            q += 2;
            if (0xDC00 <= ch2 && ch2 <= 0xDFFF) {
#ifndef Py_UNICODE_WIDE
                *p++ = ch;
                *p++ = ch2;
#else
                *p++ = (((ch & 0x3FF)<<10) | (ch2 & 0x3FF)) + 0x10000;
#endif
                continue;
            }
            else {
                errmsg = "illegal UTF-16 surrogate";
                startinpos = (((const char *)q)-4)-starts;
                endinpos = startinpos+2;
                goto utf16Error;
            }

        }
        errmsg = "illegal encoding";
        startinpos = (((const char *)q)-2)-starts;
        endinpos = startinpos+2;
        /* Fall through to report the error */

      utf16Error:
        outpos = p - PyUnicode_AS_UNICODE(unicode);
        if (unicode_decode_call_errorhandler(
                errors,
                &errorHandler,
                "utf16", errmsg,
                &starts,
                (const char **)&e,
                &startinpos,
                &endinpos,
                &exc,
                (const char **)&q,
                &unicode,
                &outpos,
                &p))
            goto onError;
    }
                while (_q < aligned_end) {
                    unsigned long data = * (unsigned long *) _q;
                    if (data & SWAPPED_FAST_CHAR_MASK)
                        break;
                    /* Zero upper bytes in UCS-4 builds */
#if (Py_UNICODE_SIZE > 2)
                    _p[0] = 0;
                    _p[1] = 0;
#if (SIZEOF_LONG == 8)
                    _p[2] = 0;
                    _p[3] = 0;
#endif
#endif
                    /* Issue #4916; UCS-4 builds on big endian machines must
                       fill the two last bytes of each 4-byte unit. */
#if (!defined(BYTEORDER_IS_LITTLE_ENDIAN) && Py_UNICODE_SIZE > 2)
# define OFF 2
#else
# define OFF 0
#endif
                    ((unsigned char *) _p)[OFF + 1] = _q[0];
                    ((unsigned char *) _p)[OFF + 0] = _q[1];
                    ((unsigned char *) _p)[OFF + 1 + Py_UNICODE_SIZE] = _q[2];
                    ((unsigned char *) _p)[OFF + 0 + Py_UNICODE_SIZE] = _q[3];
#if (SIZEOF_LONG == 8)
                    ((unsigned char *) _p)[OFF + 1 + 2 * Py_UNICODE_SIZE] = _q[4];
                    ((unsigned char *) _p)[OFF + 0 + 2 * Py_UNICODE_SIZE] = _q[5];
                    ((unsigned char *) _p)[OFF + 1 + 3 * Py_UNICODE_SIZE] = _q[6];
                    ((unsigned char *) _p)[OFF + 0 + 3 * Py_UNICODE_SIZE] = _q[7];
#endif
#undef OFF
                    _q += SIZEOF_LONG;
                    _p += SIZEOF_LONG / 2;
                }
            }
            p = _p;
            q = _q;
            if (q >= e)
                break;
        }
        ch = (q[ihi] << 8) | q[ilo];

        q += 2;

        if (ch < 0xD800 || ch > 0xDFFF) {
            *p++ = ch;
            continue;
        }

        /* UTF-16 code pair: */
        if (q > e) {
            errmsg = "unexpected end of data";
            startinpos = (((const char *)q) - 2) - starts;
            endinpos = ((const char *)e) + 1 - starts;
            goto utf16Error;
        }
        if (0xD800 <= ch && ch <= 0xDBFF) {
            Py_UNICODE ch2 = (q[ihi] << 8) | q[ilo];
            q += 2;
            if (0xDC00 <= ch2 && ch2 <= 0xDFFF) {
#ifndef Py_UNICODE_WIDE
                *p++ = ch;
                *p++ = ch2;
#else
                *p++ = (((ch & 0x3FF)<<10) | (ch2 & 0x3FF)) + 0x10000;
#endif
                continue;
            }
        if (ch < 0xD800 || ch > 0xDFFF) {
            *p++ = ch;
            continue;
        }

        /* UTF-16 code pair: */
        if (q > e) {
            errmsg = "unexpected end of data";
            startinpos = (((const char *)q) - 2) - starts;
            endinpos = ((const char *)e) + 1 - starts;
            goto utf16Error;
        }
            else {
                errmsg = "illegal UTF-16 surrogate";
                startinpos = (((const char *)q)-4)-starts;
                endinpos = startinpos+2;
                goto utf16Error;
            }

        }
        errmsg = "illegal encoding";
        startinpos = (((const char *)q)-2)-starts;
        endinpos = startinpos+2;
        /* Fall through to report the error */

      utf16Error:
        outpos = p - PyUnicode_AS_UNICODE(unicode);
        if (unicode_decode_call_errorhandler(
                errors,
                &errorHandler,
                "utf16", errmsg,
                &starts,
                (const char **)&e,
                &startinpos,
                &endinpos,
                &exc,
                (const char **)&q,
                &unicode,
                &outpos,
                &p))
            goto onError;
    }
    /* remaining byte at the end? (size should be even) */
    if (e == q) {
        if (!consumed) {
            errmsg = "truncated data";
            startinpos = ((const char *)q) - starts;
            endinpos = ((const char *)e) + 1 - starts;
            outpos = p - PyUnicode_AS_UNICODE(unicode);
            if (unicode_decode_call_errorhandler(
                    errors,
                    &errorHandler,
                    "utf16", errmsg,
                    &starts,
                    (const char **)&e,
                    &startinpos,
                    &endinpos,
                    &exc,
                    (const char **)&q,
                    &unicode,
                    &outpos,
                    &p))
                goto onError;
            /* The remaining input chars are ignored if the callback
               chooses to skip the input */
        }