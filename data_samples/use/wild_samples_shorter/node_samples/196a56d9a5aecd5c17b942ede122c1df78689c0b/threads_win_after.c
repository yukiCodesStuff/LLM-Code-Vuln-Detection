/*
 * Copyright 2016-2022 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
# endif
#endif

/*
 * VC++ 2008 or earlier x86 compilers do not have an inline implementation
 * of InterlockedOr64 for 32bit and will fail to run on Windows XP 32bit.
 * https://docs.microsoft.com/en-us/cpp/intrinsics/interlockedor-intrinsic-functions#requirements
 * To work around this problem, we implement a manual locking mechanism for
 * only VC++ 2008 or earlier x86 compilers.
 */

#if (defined(_MSC_VER) && defined(_M_IX86) && _MSC_VER <= 1500)
# define NO_INTERLOCKEDOR64
#endif

#include <openssl/crypto.h>

#if defined(OPENSSL_THREADS) && !defined(CRYPTO_TDEBUG) && defined(OPENSSL_SYS_WINDOWS)

int CRYPTO_atomic_or(uint64_t *val, uint64_t op, uint64_t *ret,
                     CRYPTO_RWLOCK *lock)
{
#if (defined(NO_INTERLOCKEDOR64))
    if (lock == NULL || !CRYPTO_THREAD_write_lock(lock))
        return 0;
    *val |= op;
    *ret = *val;

    if (!CRYPTO_THREAD_unlock(lock))
        return 0;

    return 1;
#else
    *ret = (uint64_t)InterlockedOr64((LONG64 volatile *)val, (LONG64)op) | op;
    return 1;
#endif
}

int CRYPTO_atomic_load(uint64_t *val, uint64_t *ret, CRYPTO_RWLOCK *lock)
{
#if (defined(NO_INTERLOCKEDOR64))
    if (lock == NULL || !CRYPTO_THREAD_read_lock(lock))
        return 0;
    *ret = *val;
    if (!CRYPTO_THREAD_unlock(lock))
        return 0;

    return 1;
#else
    *ret = (uint64_t)InterlockedOr64((LONG64 volatile *)val, 0);
    return 1;
#endif
}

int openssl_init_fork_handlers(void)
{