/*
 * Copyright 2019-2022 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
    unsigned int bias = initial_bias;
    size_t processed_in = 0, written_out = 0;
    unsigned int max_out = *pout_length;
    unsigned int basic_count = 0;
    unsigned int loop;

    for (loop = 0; loop < enc_len; loop++) {
        n = n + i / (written_out + 1);
        i %= (written_out + 1);

        if (written_out >= max_out)
            return 0;

        memmove(pDecoded + i + 1, pDecoded + i,
                (written_out - i) * sizeof(*pDecoded));
        pDecoded[i] = n;
        i++;
        written_out++;
    }
     */
    char *outptr = out;
    const char *inptr = in;
    size_t size = 0, maxsize;
    int result = 1;
    unsigned int i, j;
    unsigned int buf[LABEL_BUF_SIZE];      /* It's a hostname */

    if (out == NULL) {
        result = 0;
        maxsize = 0;
    } else {
        maxsize = *outlen;
    }

#define PUSHC(c)                    \
    do                              \
        if (size++ < maxsize)       \
            *outptr++ = c;          \
        else                        \
            result = 0;             \
    while (0)

    while (1) {
        char *tmpptr = strchr(inptr, '.');
        size_t delta = tmpptr != NULL ? (size_t)(tmpptr - inptr) : strlen(inptr);

        if (strncmp(inptr, "xn--", 4) != 0) {
            for (i = 0; i < delta + 1; i++)
                PUSHC(inptr[i]);
        } else {
            unsigned int bufsize = LABEL_BUF_SIZE;

            if (ossl_punycode_decode(inptr + 4, delta - 4, buf, &bufsize) <= 0)
                return -1;

            for (i = 0; i < bufsize; i++) {
                unsigned char seed[6];
                size_t utfsize = codepoint2utf8(seed, buf[i]);

                if (utfsize == 0)
                    return -1;

                for (j = 0; j < utfsize; j++)
                    PUSHC(seed[j]);
            }

            PUSHC(tmpptr != NULL ? '.' : '\0');
        }

        if (tmpptr == NULL)
            break;

        inptr = tmpptr + 1;
    }
#undef PUSHC

    *outlen = size;
    return result;
}

/*-

int ossl_a2ucompare(const char *a, const char *u)
{
    char a_ulabel[LABEL_BUF_SIZE + 1];
    size_t a_size = sizeof(a_ulabel);

    if (ossl_a2ulabel(a, a_ulabel, &a_size) <= 0)
        return -1;

    return strcmp(a_ulabel, u) != 0;
}