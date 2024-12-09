    /* Unpack UTF-16 encoded data */
    p = unicode->str;
    q = (unsigned char *)s;
    e = q + size;

    if (byteorder)
        bo = *byteorder;

#endif

    aligned_end = (const unsigned char *) ((size_t) e & ~LONG_PTR_MASK);
    while (1) {
        Py_UNICODE ch;
        if (e - q < 2) {
            /* remaining byte at the end? (size should be even) */
            if (q == e || consumed)
                break;
            errmsg = "truncated data";
            startinpos = ((const char *)q) - starts;
            endinpos = ((const char *)e) - starts;
            outpos = p - PyUnicode_AS_UNICODE(unicode);
            goto utf16Error;
            /* The remaining input chars are ignored if the callback
               chooses to skip the input */
        }
        /* First check for possible aligned read of a C 'long'. Unaligned
           reads are more expensive, better to defer to another iteration. */
        if (!((size_t) q & LONG_PTR_MASK)) {
            /* Fast path for runs of non-surrogate chars. */
            }
            p = _p;
            q = _q;
            if (e - q < 2)
                continue;
        }
        ch = (q[ihi] << 8) | q[ilo];

        q += 2;
        }

        /* UTF-16 code pair: */
        if (e - q < 2) {
            errmsg = "unexpected end of data";
            startinpos = (((const char *)q) - 2) - starts;
            endinpos = ((const char *)e) - starts;
            goto utf16Error;
        }
        if (0xD800 <= ch && ch <= 0xDBFF) {
            Py_UNICODE ch2 = (q[ihi] << 8) | q[ilo];
                &outpos,
                &p))
            goto onError;
        /* Update data because unicode_decode_call_errorhandler might have
           changed the input object. */
        aligned_end = (const unsigned char *) ((size_t) e & ~LONG_PTR_MASK);
    }

    if (byteorder)
        *byteorder = bo;