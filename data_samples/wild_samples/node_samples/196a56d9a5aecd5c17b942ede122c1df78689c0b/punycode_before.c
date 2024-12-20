/*
 * Copyright 2019-2020 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <openssl/e_os2.h>
#include "crypto/punycode.h"

static const unsigned int base = 36;
static const unsigned int tmin = 1;
static const unsigned int tmax = 26;
static const unsigned int skew = 38;
static const unsigned int damp = 700;
static const unsigned int initial_bias = 72;
static const unsigned int initial_n = 0x80;
static const unsigned int maxint = 0xFFFFFFFF;
static const char delimiter = '-';

#define LABEL_BUF_SIZE 512

/*-
 * Pseudocode:
 *
 * function adapt(delta,numpoints,firsttime):
 *  if firsttime then let delta = delta div damp
 *  else let delta = delta div 2
 *  let delta = delta + (delta div numpoints)
 *  let k = 0
 *  while delta > ((base - tmin) * tmax) div 2 do begin
 *    let delta = delta div (base - tmin)
 *    let k = k + base
 *  end
 *  return k + (((base - tmin + 1) * delta) div (delta + skew))
 */

static int adapt(unsigned int delta, unsigned int numpoints,
                 unsigned int firsttime)
{
    unsigned int k = 0;

    delta = (firsttime) ? delta / damp : delta / 2;
    delta = delta + delta / numpoints;

    while (delta > ((base - tmin) * tmax) / 2) {
        delta = delta / (base - tmin);
        k = k + base;
    }

    return k + (((base - tmin + 1) * delta) / (delta + skew));
}
{
    unsigned int n = initial_n;
    unsigned int i = 0;
    unsigned int bias = initial_bias;
    size_t processed_in = 0, written_out = 0;
    unsigned int max_out = *pout_length;

    unsigned int basic_count = 0;
    unsigned int loop;

    for (loop = 0; loop < enc_len; loop++) {
        if (pEncoded[loop] == delimiter)
            basic_count = loop;
    }
        for (k = base;; k += base) {
            if (loop >= enc_len)
                return 0;

            digit = digit_decoded(pEncoded[loop]);
            loop++;

            if (digit < 0)
                return 0;
            if ((unsigned int)digit > (maxint - i) / w)
                return 0;

            i = i + digit * w;
            t = (k <= bias) ? tmin : (k >= bias + tmax) ? tmax : k - bias;

            if ((unsigned int)digit < t)
                break;

            if (w > maxint / (base - t))
                return 0;
            w = w * (base - t);
        }

        bias = adapt(i - oldi, written_out + 1, (oldi == 0));
        if (i / (written_out + 1) > maxint - n)
            return 0;
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

    *pout_length = written_out;
    return 1;
}

/*
 * Encode a code point using UTF-8
 * return number of bytes on success, 0 on failure
 * (also produces U+FFFD, which uses 3 bytes on failure)
 */
static int codepoint2utf8(unsigned char *out, unsigned long utf)
{
    if (utf <= 0x7F) {
        /* Plain ASCII */
        out[0] = (unsigned char)utf;
        out[1] = 0;
        return 1;
    } else if (utf <= 0x07FF) {
        /* 2-byte unicode */
        out[0] = (unsigned char)(((utf >> 6) & 0x1F) | 0xC0);
        out[1] = (unsigned char)(((utf >> 0) & 0x3F) | 0x80);
        out[2] = 0;
        return 2;
    } else if (utf <= 0xFFFF) {
        /* 3-byte unicode */
        out[0] = (unsigned char)(((utf >> 12) & 0x0F) | 0xE0);
        out[1] = (unsigned char)(((utf >> 6) & 0x3F) | 0x80);
        out[2] = (unsigned char)(((utf >> 0) & 0x3F) | 0x80);
        out[3] = 0;
        return 3;
    } else if (utf <= 0x10FFFF) {
        /* 4-byte unicode */
        out[0] = (unsigned char)(((utf >> 18) & 0x07) | 0xF0);
        out[1] = (unsigned char)(((utf >> 12) & 0x3F) | 0x80);
        out[2] = (unsigned char)(((utf >> 6) & 0x3F) | 0x80);
        out[3] = (unsigned char)(((utf >> 0) & 0x3F) | 0x80);
        out[4] = 0;
        return 4;
    } else {
        /* error - use replacement character */
        out[0] = (unsigned char)0xEF;
        out[1] = (unsigned char)0xBF;
        out[2] = (unsigned char)0xBD;
        out[3] = 0;
        return 0;
    }
}

/*-
 * Return values:
 * 1 - ok, *outlen contains valid buf length
 * 0 - ok but buf was too short, *outlen contains valid buf length
 * -1 - bad string passed
 */

int ossl_a2ulabel(const char *in, char *out, size_t *outlen)
{
    /*-
     * Domain name has some parts consisting of ASCII chars joined with dot.
     * If a part is shorter than 5 chars, it becomes U-label as is.
     * If it does not start with xn--,    it becomes U-label as is.
     * Otherwise we try to decode it.
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
 * a MUST be A-label
 * u MUST be U-label
 * Returns 0 if compared values are equal
 * 1 if not
 * -1 in case of errors
 */

int ossl_a2ucompare(const char *a, const char *u)
{
    char a_ulabel[LABEL_BUF_SIZE];
    size_t a_size = sizeof(a_ulabel);

    if (ossl_a2ulabel(a, a_ulabel, &a_size) <= 0) {
        return -1;
    }

    return (strcmp(a_ulabel, u) == 0) ? 0 : 1;
}
{
    /*-
     * Domain name has some parts consisting of ASCII chars joined with dot.
     * If a part is shorter than 5 chars, it becomes U-label as is.
     * If it does not start with xn--,    it becomes U-label as is.
     * Otherwise we try to decode it.
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
 * a MUST be A-label
 * u MUST be U-label
 * Returns 0 if compared values are equal
 * 1 if not
 * -1 in case of errors
 */

int ossl_a2ucompare(const char *a, const char *u)
{
    char a_ulabel[LABEL_BUF_SIZE];
    size_t a_size = sizeof(a_ulabel);

    if (ossl_a2ulabel(a, a_ulabel, &a_size) <= 0) {
        return -1;
    }

    return (strcmp(a_ulabel, u) == 0) ? 0 : 1;
}