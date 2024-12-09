/*
 * Copyright 2001-2021 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
    return ossl_rand_pool_entropy_available(pool);
}

int ossl_pool_add_nonce_data(RAND_POOL *pool)
{
    struct {
        pid_t pid;
        CRYPTO_THREAD_ID tid;
        unsigned __int64 time;
    } data;

    /* Erase the entire structure including any padding */
    memset(&data, 0, sizeof(data));

    /*
     * Add process id, thread id, and a high resolution timestamp
     * (where available, which is OpenVMS v8.4 and up) to ensure that
     * the nonce is unique with high probability for different process
     * instances.
     */
    data.pid = getpid();
    data.tid = CRYPTO_THREAD_get_current_id();
#if __CRTL_VER >= 80400000
    sys$gettim_prec(&data.time);
#else
    sys$gettim((void*)&data.time);
#endif

    return ossl_rand_pool_add(pool, (unsigned char *)&data, sizeof(data), 0);
}

/*
 * SYS$GET_ENTROPY METHOD
 * ======================
 *
    return data_collect_method(pool);
}


int ossl_rand_pool_add_additional_data(RAND_POOL *pool)
{
    struct {
        CRYPTO_THREAD_ID tid;
        unsigned __int64 time;
    } data;

    /* Erase the entire structure including any padding */
    memset(&data, 0, sizeof(data));

    /*
     * Add some noise from the thread id and a high resolution timer.
     * The thread id adds a little randomness if the drbg is accessed
     * concurrently (which is the case for the <master> drbg).
     */
    data.tid = CRYPTO_THREAD_get_current_id();
#if __CRTL_VER >= 80400000
    sys$gettim_prec(&data.time);
#else
    sys$gettim((void*)&data.time);
#endif

    return ossl_rand_pool_add(pool, (unsigned char *)&data, sizeof(data), 0);
}
