    /* Unpack UTF-16 encoded data */
    p = unicode->str;
    q = (unsigned char *)s;
    e = q + size - 1;

    if (byteorder)
        bo = *byteorder;

#endif

    aligned_end = (const unsigned char *) ((size_t) e & ~LONG_PTR_MASK);
    while (q < e) {
        Py_UNICODE ch;
        /* First check for possible aligned read of a C 'long'. Unaligned
           reads are more expensive, better to defer to another iteration. */
        if (!((size_t) q & LONG_PTR_MASK)) {
            /* Fast path for runs of non-surrogate chars. */
            }
            p = _p;
            q = _q;
            if (q >= e)
                break;
        }
        ch = (q[ihi] << 8) | q[ilo];

        q += 2;
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
    }

    if (byteorder)
        *byteorder = bo;