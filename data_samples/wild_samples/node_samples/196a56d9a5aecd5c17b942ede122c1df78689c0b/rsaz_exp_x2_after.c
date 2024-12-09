/*
 * Copyright 2020-2022 The OpenSSL Project Authors. All Rights Reserved.
 * Copyright (c) 2020, Intel Corporation. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 *
 *
 * Originally written by Ilya Albrekht, Sergey Kirillov and Andrey Matyukov
 * Intel Corporation
 *
 */

#include <openssl/opensslconf.h>
#include <openssl/crypto.h>
#include "rsaz_exp.h"

#ifndef RSAZ_ENABLED
NON_EMPTY_TRANSLATION_UNIT
#else
# include <assert.h>
# include <string.h>

# if defined(__GNUC__)
#  define ALIGN64 __attribute__((aligned(64)))
# elif defined(_MSC_VER)
#  define ALIGN64 __declspec(align(64))
# else
#  define ALIGN64
# endif

# if defined(__GNUC__)
#  define ALIGN1  __attribute__((aligned(1)))
# elif defined(_MSC_VER)
#  define ALIGN1  __declspec(align(1))
# else
#  define ALIGN1
# endif

# define ALIGN_OF(ptr, boundary) \
    ((unsigned char *)(ptr) + (boundary - (((size_t)(ptr)) & (boundary - 1))))

/* Internal radix */
# define DIGIT_SIZE (52)
/* 52-bit mask */
# define DIGIT_MASK ((uint64_t)0xFFFFFFFFFFFFF)

# define BITS2WORD8_SIZE(x)  (((x) + 7) >> 3)
# define BITS2WORD64_SIZE(x) (((x) + 63) >> 6)

typedef uint64_t ALIGN1 uint64_t_align1;

static ossl_inline uint64_t get_digit52(const uint8_t *in, int in_len);
static ossl_inline void put_digit52(uint8_t *out, int out_len, uint64_t digit);
static void to_words52(BN_ULONG *out, int out_len, const BN_ULONG *in,
                       int in_bitsize);
static void from_words52(BN_ULONG *bn_out, int out_bitsize, const BN_ULONG *in);
static ossl_inline void set_bit(BN_ULONG *a, int idx);

/* Number of |digit_size|-bit digits in |bitsize|-bit value */
static ossl_inline int number_of_digits(int bitsize, int digit_size)
{
    return (bitsize + digit_size - 1) / digit_size;
}
/*
 * Copyright 2020-2022 The OpenSSL Project Authors. All Rights Reserved.
 * Copyright (c) 2020, Intel Corporation. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 *
 *
 * Originally written by Ilya Albrekht, Sergey Kirillov and Andrey Matyukov
 * Intel Corporation
 *
 */

#include <openssl/opensslconf.h>
#include <openssl/crypto.h>
#include "rsaz_exp.h"

#ifndef RSAZ_ENABLED
NON_EMPTY_TRANSLATION_UNIT
#else
# include <assert.h>
# include <string.h>

# if defined(__GNUC__)
#  define ALIGN64 __attribute__((aligned(64)))
# elif defined(_MSC_VER)
#  define ALIGN64 __declspec(align(64))
# else
#  define ALIGN64
# endif

# if defined(__GNUC__)
#  define ALIGN1  __attribute__((aligned(1)))
# elif defined(_MSC_VER)
#  define ALIGN1  __declspec(align(1))
# else
#  define ALIGN1
# endif

# define ALIGN_OF(ptr, boundary) \
    ((unsigned char *)(ptr) + (boundary - (((size_t)(ptr)) & (boundary - 1))))

/* Internal radix */
# define DIGIT_SIZE (52)
/* 52-bit mask */
# define DIGIT_MASK ((uint64_t)0xFFFFFFFFFFFFF)

# define BITS2WORD8_SIZE(x)  (((x) + 7) >> 3)
# define BITS2WORD64_SIZE(x) (((x) + 63) >> 6)

typedef uint64_t ALIGN1 uint64_t_align1;

static ossl_inline uint64_t get_digit52(const uint8_t *in, int in_len);
static ossl_inline void put_digit52(uint8_t *out, int out_len, uint64_t digit);
static void to_words52(BN_ULONG *out, int out_len, const BN_ULONG *in,
                       int in_bitsize);
static void from_words52(BN_ULONG *bn_out, int out_bitsize, const BN_ULONG *in);
static ossl_inline void set_bit(BN_ULONG *a, int idx);

/* Number of |digit_size|-bit digits in |bitsize|-bit value */
static ossl_inline int number_of_digits(int bitsize, int digit_size)
{
    return (bitsize + digit_size - 1) / digit_size;
}
{
    uint8_t *in_str = NULL;

    assert(out != NULL);
    assert(in != NULL);
    /* Check destination buffer capacity */
    assert(out_len >= number_of_digits(in_bitsize, DIGIT_SIZE));

    in_str = (uint8_t *)in;

    for (; in_bitsize >= (2 * DIGIT_SIZE); in_bitsize -= (2 * DIGIT_SIZE), out += 2) {
        out[0] = (*(uint64_t_align1 *)in_str) & DIGIT_MASK;
        in_str += 6;
        out[1] = ((*(uint64_t_align1 *)in_str) >> 4) & DIGIT_MASK;
        in_str += 7;
        out_len -= 2;
    }
    {
        uint8_t *out_str = (uint8_t *)out;

        for (; out_bitsize >= (2 * DIGIT_SIZE); out_bitsize -= (2 * DIGIT_SIZE), in += 2) {
            (*(uint64_t_align1 *)out_str) = in[0];
            out_str += 6;
            (*(uint64_t_align1 *)out_str) ^= in[1] << 4;
            out_str += 7;
        }