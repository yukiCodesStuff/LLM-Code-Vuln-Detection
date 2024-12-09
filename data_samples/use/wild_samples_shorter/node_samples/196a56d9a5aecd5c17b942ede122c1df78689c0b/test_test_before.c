/*
 * Copyright 2017-2018 The OpenSSL Project Authors. All Rights Reserved.
 * Copyright (c) 2017, Oracle and/or its affiliates.  All rights reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
static int test_int(void)
{
    if (!TEST(1, TEST_int_eq(1, 1))
        | !TEST(0, TEST_int_eq(1, -1))
        | !TEST(1, TEST_int_ne(1, 2))
        | !TEST(0, TEST_int_ne(3, 3))
        | !TEST(1, TEST_int_lt(4, 9))
        | !TEST(0, TEST_int_lt(9, 4))
        | !TEST(1, TEST_int_le(4, 9))
        | !TEST(1, TEST_int_le(5, 5))
        | !TEST(0, TEST_int_le(9, 4))
        | !TEST(1, TEST_int_gt(8, 5))
        | !TEST(0, TEST_int_gt(5, 8))
        | !TEST(1, TEST_int_ge(8, 5))
        | !TEST(1, TEST_int_ge(6, 6))
        | !TEST(0, TEST_int_ge(5, 8)))
        goto err;
    return 1;

err:
static int test_uint(void)
{
    if (!TEST(1, TEST_uint_eq(3u, 3u))
        | !TEST(0, TEST_uint_eq(3u, 5u))
        | !TEST(1, TEST_uint_ne(4u, 2u))
        | !TEST(0, TEST_uint_ne(6u, 6u))
        | !TEST(1, TEST_uint_lt(5u, 9u))
        | !TEST(0, TEST_uint_lt(9u, 5u))
        | !TEST(1, TEST_uint_le(5u, 9u))
        | !TEST(1, TEST_uint_le(7u, 7u))
        | !TEST(0, TEST_uint_le(9u, 5u))
        | !TEST(1, TEST_uint_gt(11u, 1u))
        | !TEST(0, TEST_uint_gt(1u, 11u))
        | !TEST(1, TEST_uint_ge(11u, 1u))
        | !TEST(1, TEST_uint_ge(6u, 6u))
        | !TEST(0, TEST_uint_ge(1u, 11u)))
        goto err;
    return 1;

err:
static int test_char(void)
{
    if (!TEST(1, TEST_char_eq('a', 'a'))
        | !TEST(0, TEST_char_eq('a', 'A'))
        | !TEST(1, TEST_char_ne('a', 'c'))
        | !TEST(0, TEST_char_ne('e', 'e'))
        | !TEST(1, TEST_char_lt('i', 'x'))
        | !TEST(0, TEST_char_lt('x', 'i'))
        | !TEST(1, TEST_char_le('i', 'x'))
        | !TEST(1, TEST_char_le('n', 'n'))
        | !TEST(0, TEST_char_le('x', 'i'))
        | !TEST(1, TEST_char_gt('w', 'n'))
        | !TEST(0, TEST_char_gt('n', 'w'))
        | !TEST(1, TEST_char_ge('w', 'n'))
        | !TEST(1, TEST_char_ge('p', 'p'))
        | !TEST(0, TEST_char_ge('n', 'w')))
        goto err;
    return 1;

err:
static int test_uchar(void)
{
    if (!TEST(1, TEST_uchar_eq(49, 49))
        | !TEST(0, TEST_uchar_eq(49, 60))
        | !TEST(1, TEST_uchar_ne(50, 2))
        | !TEST(0, TEST_uchar_ne(66, 66))
        | !TEST(1, TEST_uchar_lt(60, 80))
        | !TEST(0, TEST_uchar_lt(80, 60))
        | !TEST(1, TEST_uchar_le(60, 80))
        | !TEST(1, TEST_uchar_le(78, 78))
        | !TEST(0, TEST_uchar_le(80, 60))
        | !TEST(1, TEST_uchar_gt(88, 37))
        | !TEST(0, TEST_uchar_gt(37, 88))
        | !TEST(1, TEST_uchar_ge(88, 37))
        | !TEST(1, TEST_uchar_ge(66, 66))
        | !TEST(0, TEST_uchar_ge(37, 88)))
        goto err;
    return 1;

err:
static int test_long(void)
{
    if (!TEST(1, TEST_long_eq(123l, 123l))
        | !TEST(0, TEST_long_eq(123l, -123l))
        | !TEST(1, TEST_long_ne(123l, 500l))
        | !TEST(0, TEST_long_ne(1000l, 1000l))
        | !TEST(1, TEST_long_lt(-8923l, 102934563l))
        | !TEST(0, TEST_long_lt(102934563l, -8923l))
        | !TEST(1, TEST_long_le(-8923l, 102934563l))
        | !TEST(1, TEST_long_le(12345l, 12345l))
        | !TEST(0, TEST_long_le(102934563l, -8923l))
        | !TEST(1, TEST_long_gt(84325677l, 12345l))
        | !TEST(0, TEST_long_gt(12345l, 84325677l))
        | !TEST(1, TEST_long_ge(84325677l, 12345l))
        | !TEST(1, TEST_long_ge(465869l, 465869l))
        | !TEST(0, TEST_long_ge(12345l, 84325677l)))
        goto err;
    return 1;

err:
static int test_ulong(void)
{
    if (!TEST(1, TEST_ulong_eq(919ul, 919ul))
        | !TEST(0, TEST_ulong_eq(919ul, 10234ul))
        | !TEST(1, TEST_ulong_ne(8190ul, 66ul))
        | !TEST(0, TEST_ulong_ne(10555ul, 10555ul))
        | !TEST(1, TEST_ulong_lt(10234ul, 1000000ul))
        | !TEST(0, TEST_ulong_lt(1000000ul, 10234ul))
        | !TEST(1, TEST_ulong_le(10234ul, 1000000ul))
        | !TEST(1, TEST_ulong_le(100000ul, 100000ul))
        | !TEST(0, TEST_ulong_le(1000000ul, 10234ul))
        | !TEST(1, TEST_ulong_gt(100000000ul, 22ul))
        | !TEST(0, TEST_ulong_gt(22ul, 100000000ul))
        | !TEST(1, TEST_ulong_ge(100000000ul, 22ul))
        | !TEST(1, TEST_ulong_ge(10555ul, 10555ul))
        | !TEST(0, TEST_ulong_ge(22ul, 100000000ul)))
        goto err;
    return 1;

err:
static int test_size_t(void)
{
    if (!TEST(1, TEST_size_t_eq((size_t)10, (size_t)10))
        | !TEST(0, TEST_size_t_eq((size_t)10, (size_t)12))
        | !TEST(1, TEST_size_t_ne((size_t)10, (size_t)12))
        | !TEST(0, TEST_size_t_ne((size_t)24, (size_t)24))
        | !TEST(1, TEST_size_t_lt((size_t)30, (size_t)88))
        | !TEST(0, TEST_size_t_lt((size_t)88, (size_t)30))
        | !TEST(1, TEST_size_t_le((size_t)30, (size_t)88))
        | !TEST(1, TEST_size_t_le((size_t)33, (size_t)33))
        | !TEST(0, TEST_size_t_le((size_t)88, (size_t)30))
        | !TEST(1, TEST_size_t_gt((size_t)52, (size_t)33))
        | !TEST(0, TEST_size_t_gt((size_t)33, (size_t)52))
        | !TEST(1, TEST_size_t_ge((size_t)52, (size_t)33))
        | !TEST(1, TEST_size_t_ge((size_t)38, (size_t)38))
        | !TEST(0, TEST_size_t_ge((size_t)33, (size_t)52)))
        goto err;
    return 1;

err:
static int test_time_t(void)
{
    if (!TEST(1, TEST_time_t_eq((time_t)10, (time_t)10))
        | !TEST(0, TEST_time_t_eq((time_t)10, (time_t)12))
        | !TEST(1, TEST_time_t_ne((time_t)10, (time_t)12))
        | !TEST(0, TEST_time_t_ne((time_t)24, (time_t)24))
        | !TEST(1, TEST_time_t_lt((time_t)30, (time_t)88))
        | !TEST(0, TEST_time_t_lt((time_t)88, (time_t)30))
        | !TEST(1, TEST_time_t_le((time_t)30, (time_t)88))
        | !TEST(1, TEST_time_t_le((time_t)33, (time_t)33))
        | !TEST(0, TEST_time_t_le((time_t)88, (time_t)30))
        | !TEST(1, TEST_time_t_gt((time_t)52, (time_t)33))
        | !TEST(0, TEST_time_t_gt((time_t)33, (time_t)52))
        | !TEST(1, TEST_time_t_ge((time_t)52, (time_t)33))
        | !TEST(1, TEST_time_t_ge((time_t)38, (time_t)38))
        | !TEST(0, TEST_time_t_ge((time_t)33, (time_t)52)))
        goto err;
    return 1;

err:
    char y = 1;

    if (!TEST(1, TEST_ptr(&y))
        | !TEST(0, TEST_ptr(NULL))
        | !TEST(0, TEST_ptr_null(&y))
        | !TEST(1, TEST_ptr_null(NULL))
        | !TEST(1, TEST_ptr_eq(NULL, NULL))
        | !TEST(0, TEST_ptr_eq(NULL, &y))
        | !TEST(0, TEST_ptr_eq(&y, NULL))
        | !TEST(0, TEST_ptr_eq(&y, &x))
        | !TEST(1, TEST_ptr_eq(&x, &x))
        | !TEST(0, TEST_ptr_ne(NULL, NULL))
        | !TEST(1, TEST_ptr_ne(NULL, &y))
        | !TEST(1, TEST_ptr_ne(&y, NULL))
        | !TEST(1, TEST_ptr_ne(&y, &x))
        | !TEST(0, TEST_ptr_ne(&x, &x)))
        goto err;
    return 1;

err:
static int test_bool(void)
{
    if (!TEST(0, TEST_true(0))
        | !TEST(1, TEST_true(1))
        | !TEST(1, TEST_false(0))
        | !TEST(0, TEST_false(1)))
        goto err;
    return 1;

err:
    static char buf[] = "abc";

    if (!TEST(1, TEST_str_eq(NULL, NULL))
        | !TEST(1, TEST_str_eq("abc", buf))
        | !TEST(0, TEST_str_eq("abc", NULL))
        | !TEST(0, TEST_str_eq("abc", ""))
        | !TEST(0, TEST_str_eq(NULL, buf))
        | !TEST(0, TEST_str_ne(NULL, NULL))
        | !TEST(0, TEST_str_eq("", NULL))
        | !TEST(0, TEST_str_eq(NULL, ""))
        | !TEST(0, TEST_str_ne("", ""))
        | !TEST(0, TEST_str_eq("\1\2\3\4\5", "\1x\3\6\5"))
        | !TEST(0, TEST_str_ne("abc", buf))
        | !TEST(1, TEST_str_ne("abc", NULL))
        | !TEST(1, TEST_str_ne(NULL, buf))
        | !TEST(0, TEST_str_eq("abcdef", "abcdefghijk")))
        goto err;
    return 1;

err:
    static char buf[] = "xyz";

    if (!TEST(1, TEST_mem_eq(NULL, 0, NULL, 0))
        | !TEST(1, TEST_mem_eq(NULL, 1, NULL, 2))
        | !TEST(0, TEST_mem_eq(NULL, 0, "xyz", 3))
        | !TEST(0, TEST_mem_eq(NULL, 7, "abc", 3))
        | !TEST(0, TEST_mem_ne(NULL, 0, NULL, 0))
        | !TEST(0, TEST_mem_eq(NULL, 0, "", 0))
        | !TEST(0, TEST_mem_eq("", 0, NULL, 0))
        | !TEST(0, TEST_mem_ne("", 0, "", 0))
        | !TEST(0, TEST_mem_eq("xyz", 3, NULL, 0))
        | !TEST(0, TEST_mem_eq("xyz", 3, buf, sizeof(buf)))
        | !TEST(1, TEST_mem_eq("xyz", 4, buf, sizeof(buf))))
        goto err;
    return 1;

err:
    int r = 0;

    if (!TEST(1, TEST_int_eq(BN_dec2bn(&a, "0"), 1))
        | !TEST(1, TEST_BN_eq_word(a, 0))
        | !TEST(0, TEST_BN_eq_word(a, 30))
        | !TEST(1, TEST_BN_abs_eq_word(a, 0))
        | !TEST(0, TEST_BN_eq_one(a))
        | !TEST(1, TEST_BN_eq_zero(a))
        | !TEST(0, TEST_BN_ne_zero(a))
        | !TEST(1, TEST_BN_le_zero(a))
        | !TEST(0, TEST_BN_lt_zero(a))
        | !TEST(1, TEST_BN_ge_zero(a))
        | !TEST(0, TEST_BN_gt_zero(a))
        | !TEST(1, TEST_BN_even(a))
        | !TEST(0, TEST_BN_odd(a))
        | !TEST(1, TEST_BN_eq(b, c))
        | !TEST(0, TEST_BN_eq(a, b))
        | !TEST(0, TEST_BN_ne(NULL, c))
        | !TEST(1, TEST_int_eq(BN_dec2bn(&b, "1"), 1))
        | !TEST(1, TEST_BN_eq_word(b, 1))
        | !TEST(1, TEST_BN_eq_one(b))
        | !TEST(0, TEST_BN_abs_eq_word(b, 0))
        | !TEST(1, TEST_BN_abs_eq_word(b, 1))
        | !TEST(0, TEST_BN_eq_zero(b))
        | !TEST(1, TEST_BN_ne_zero(b))
        | !TEST(0, TEST_BN_le_zero(b))
        | !TEST(0, TEST_BN_lt_zero(b))
        | !TEST(1, TEST_BN_ge_zero(b))
        | !TEST(1, TEST_BN_gt_zero(b))
        | !TEST(0, TEST_BN_even(b))
        | !TEST(1, TEST_BN_odd(b))
        | !TEST(1, TEST_int_eq(BN_dec2bn(&c, "-334739439"), 10))
        | !TEST(0, TEST_BN_eq_word(c, 334739439))
        | !TEST(1, TEST_BN_abs_eq_word(c, 334739439))
        | !TEST(0, TEST_BN_eq_zero(c))
        | !TEST(1, TEST_BN_ne_zero(c))
        | !TEST(1, TEST_BN_le_zero(c))
        | !TEST(1, TEST_BN_lt_zero(c))
        | !TEST(0, TEST_BN_ge_zero(c))
        | !TEST(0, TEST_BN_gt_zero(c))
        | !TEST(0, TEST_BN_even(c))
        | !TEST(1, TEST_BN_odd(c))
        | !TEST(1, TEST_BN_eq(a, a))
        | !TEST(0, TEST_BN_ne(a, a))
        | !TEST(0, TEST_BN_eq(a, b))
        | !TEST(1, TEST_BN_ne(a, b))
        | !TEST(0, TEST_BN_lt(a, c))
        | !TEST(1, TEST_BN_lt(c, b))
        | !TEST(0, TEST_BN_lt(b, c))
        | !TEST(0, TEST_BN_le(a, c))
        | !TEST(1, TEST_BN_le(c, b))
        | !TEST(0, TEST_BN_le(b, c))
        | !TEST(1, TEST_BN_gt(a, c))
        | !TEST(0, TEST_BN_gt(c, b))
        | !TEST(1, TEST_BN_gt(b, c))
        | !TEST(1, TEST_BN_ge(a, c))
        | !TEST(0, TEST_BN_ge(c, b))
        | !TEST(1, TEST_BN_ge(b, c)))
        goto err;

    r = 1;
err: