/*
 * Copyright 2019-2020 The OpenSSL Project Authors. All Rights Reserved.
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

        if (written_out > max_out)
            return 0;

        memmove(pDecoded + i + 1, pDecoded + i,
                (written_out - i) * sizeof *pDecoded);
        pDecoded[i] = n;
        i++;
        written_out++;
    }
     */
    char *outptr = out;
    const char *inptr = in;
    size_t size = 0;
    int result = 1;

    unsigned int buf[LABEL_BUF_SIZE];      /* It's a hostname */
    if (out == NULL)
        result = 0;

    while (1) {
        char *tmpptr = strchr(inptr, '.');
        size_t delta = (tmpptr) ? (size_t)(tmpptr - inptr) : strlen(inptr);

        if (strncmp(inptr, "xn--", 4) != 0) {
            size += delta + 1;

            if (size >= *outlen - 1)
                result = 0;

            if (result > 0) {
                memcpy(outptr, inptr, delta + 1);
                outptr += delta + 1;
            }
        } else {
            unsigned int bufsize = LABEL_BUF_SIZE;
            unsigned int i;

            if (ossl_punycode_decode(inptr + 4, delta - 4, buf, &bufsize) <= 0)
                return -1;

            for (i = 0; i < bufsize; i++) {
                unsigned char seed[6];
                size_t utfsize = codepoint2utf8(seed, buf[i]);
                if (utfsize == 0)
                    return -1;

                size += utfsize;
                if (size >= *outlen - 1)
                    result = 0;

                if (result > 0) {
                    memcpy(outptr, seed, utfsize);
                    outptr += utfsize;
                }
            }

            if (tmpptr != NULL) {
                *outptr = '.';
                outptr++;
                size++;
                if (size >= *outlen - 1)
                    result = 0;
            }
        }

        if (tmpptr == NULL)
            break;

        inptr = tmpptr + 1;
    }

    return result;
}

/*-

int ossl_a2ucompare(const char *a, const char *u)
{
    char a_ulabel[LABEL_BUF_SIZE];
    size_t a_size = sizeof(a_ulabel);

    if (ossl_a2ulabel(a, a_ulabel, &a_size) <= 0) {
        return -1;
    }

    return (strcmp(a_ulabel, u) == 0) ? 0 : 1;
}