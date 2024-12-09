/*
 * Copyright 2001-2022 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
    return ossl_rand_pool_entropy_available(pool);
}

/*
 * SYS$GET_ENTROPY METHOD
 * ======================
 *
    return data_collect_method(pool);
}

int ossl_pool_add_nonce_data(RAND_POOL *pool)
{
    /*
     * Two variables to ensure that two nonces won't ever be the same
     */
    static unsigned __int64 last_time = 0;
    static unsigned __int32 last_seq = 0;

    struct {
        pid_t pid;
        CRYPTO_THREAD_ID tid;
        unsigned __int64 time;
        unsigned __int32 seq;
    } data;

    /* Erase the entire structure including any padding */
    memset(&data, 0, sizeof(data));

    /*
     * Add process id, thread id, a timestamp, and a sequence number in case
     * the same time stamp is repeated, to ensure that the nonce is unique
     * with high probability for different process instances.
     *
     * The normal OpenVMS time is specified to be high granularity (100ns),
     * but the time update granularity given by sys$gettim() may be lower.
     *
     * OpenVMS version 8.4 (which is the latest for Alpha and Itanium) and
     * on have sys$gettim_prec() as well, which is supposedly having a better
     * time update granularity, but tests on Itanium (and even Alpha) have
     * shown that compared with sys$gettim(), the difference is marginal,
     * so of very little significance in terms of entropy.
     * Given that, and that it's a high ask to expect everyone to have
     * upgraded to OpenVMS version 8.4, only sys$gettim() is used, and a
     * sequence number is added as well, in case sys$gettim() returns the
     * same time value more than once.
     *
     * This function is assumed to be called under thread lock, and does
     * therefore not take concurrency into account.
     */
    data.pid = getpid();
    data.tid = CRYPTO_THREAD_get_current_id();
    data.seq = 0;
    sys$gettim((void*)&data.time);

    if (data.time == last_time) {
        data.seq = ++last_seq;
    } else {
        last_time = data.time;
        last_seq = 0;
    }

    return ossl_rand_pool_add(pool, (unsigned char *)&data, sizeof(data), 0);
}
