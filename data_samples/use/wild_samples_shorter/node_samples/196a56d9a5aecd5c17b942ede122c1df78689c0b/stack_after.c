/*
 * Copyright 1995-2022 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 */
static const int min_nodes = 4;
static const int max_nodes = SIZE_MAX / sizeof(void *) < INT_MAX
    ? (int)(SIZE_MAX / sizeof(void *)) : INT_MAX;

struct stack_st {
    int num;
    const void **data;
    OPENSSL_sk_compfunc comp;
};

OPENSSL_sk_compfunc OPENSSL_sk_set_cmp_func(OPENSSL_STACK *sk,
                                            OPENSSL_sk_compfunc c)
{
    OPENSSL_sk_compfunc old = sk->comp;

    if (sk->comp != c)
    }

    /* duplicate |sk->data| content */
    ret->data = OPENSSL_malloc(sizeof(*ret->data) * sk->num_alloc);
    if (ret->data == NULL)
        goto err;
    memcpy(ret->data, sk->data, sizeof(void *) * sk->num);
    return ret;

}

OPENSSL_STACK *OPENSSL_sk_deep_copy(const OPENSSL_STACK *sk,
                                    OPENSSL_sk_copyfunc copy_func,
                                    OPENSSL_sk_freefunc free_func)
{
    OPENSSL_STACK *ret;
    int i;

    int num_alloc;

    /* Check to see the reservation isn't exceeding the hard limit */
    if (n > max_nodes - st->num) {
        ERR_raise(ERR_LIB_CRYPTO, CRYPTO_R_TOO_MANY_RECORDS);
        return 0;
    }

    /* Figure out the new size */
    num_alloc = st->num + n;
    if (num_alloc < min_nodes)
        if (num_alloc <= st->num_alloc)
            return 1;
        num_alloc = compute_growth(num_alloc, st->num_alloc);
        if (num_alloc == 0) {
            ERR_raise(ERR_LIB_CRYPTO, CRYPTO_R_TOO_MANY_RECORDS);
            return 0;
        }
    } else if (num_alloc == st->num_alloc) {
        return 1;
    }

    tmpdata = OPENSSL_realloc((void *)st->data, sizeof(void *) * num_alloc);
    if (tmpdata == NULL) {
        ERR_raise(ERR_LIB_CRYPTO, ERR_R_MALLOC_FAILURE);
        return 0;
    }

    st->data = tmpdata;
    st->num_alloc = num_alloc;
    return 1;
{
    OPENSSL_STACK *st = OPENSSL_zalloc(sizeof(OPENSSL_STACK));

    if (st == NULL) {
        ERR_raise(ERR_LIB_CRYPTO, ERR_R_MALLOC_FAILURE);
        return NULL;
    }

    st->comp = c;

    if (n <= 0)

int OPENSSL_sk_reserve(OPENSSL_STACK *st, int n)
{
    if (st == NULL) {
        ERR_raise(ERR_LIB_CRYPTO, ERR_R_PASSED_NULL_PARAMETER);
        return 0;
    }

    if (n < 0)
        return 1;
    return sk_reserve(st, n, 1);

int OPENSSL_sk_insert(OPENSSL_STACK *st, const void *data, int loc)
{
    if (st == NULL) {
        ERR_raise(ERR_LIB_CRYPTO, ERR_R_PASSED_NULL_PARAMETER);
        return 0;
    }
    if (st->num == max_nodes) {
        ERR_raise(ERR_LIB_CRYPTO, CRYPTO_R_TOO_MANY_RECORDS);
        return 0;
    }

    if (!sk_reserve(st, 1, 0))
        return 0;

    const void *ret = st->data[loc];

    if (loc != st->num - 1)
        memmove(&st->data[loc], &st->data[loc + 1],
                sizeof(st->data[0]) * (st->num - loc - 1));
    st->num--;

    return (void *)ret;
}
{
    int i;

    if (st == NULL)
        return NULL;

    for (i = 0; i < st->num; i++)
        if (st->data[i] == p)
            return internal_delete(st, i);
    return NULL;

void *OPENSSL_sk_set(OPENSSL_STACK *st, int i, const void *data)
{
    if (st == NULL) {
        ERR_raise(ERR_LIB_CRYPTO, ERR_R_PASSED_NULL_PARAMETER);
        return NULL;
    }
    if (i < 0 || i >= st->num) {
        ERR_raise_data(ERR_LIB_CRYPTO, ERR_R_PASSED_INVALID_ARGUMENT,
                       "i=%d", i);
        return NULL;
    }
    st->data[i] = data;
    st->sorted = 0;
    return (void *)st->data[i];
}