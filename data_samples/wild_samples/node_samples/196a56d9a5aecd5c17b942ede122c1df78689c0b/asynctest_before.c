/*
 * Copyright 2015-2020 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#ifdef _WIN32
# include <windows.h>
#endif

#include <stdio.h>
#include <string.h>
#include <openssl/async.h>
#include <openssl/crypto.h>

static int ctr = 0;
static ASYNC_JOB *currjob = NULL;

static int only_pause(void *args)
{
    ASYNC_pause_job();

    return 1;
}
    if (tmpctx != globalctx) {
        fprintf(stderr,
                "test_ASYNC_start_job_ex() failed - global libctx check failed\n");
        goto err;
    }

    ret = 1;
 err:
    ASYNC_WAIT_CTX_free(waitctx);
    OSSL_LIB_CTX_free(libctx);
    return ret;
}

int main(int argc, char **argv)
{
    if (!ASYNC_is_capable()) {
        fprintf(stderr,
                "OpenSSL build is not ASYNC capable - skipping async tests\n");
    } else {
        if (!test_ASYNC_init_thread()
                || !test_ASYNC_callback_status()
                || !test_ASYNC_start_job()
                || !test_ASYNC_get_current_job()
                || !test_ASYNC_WAIT_CTX_get_all_fds()
                || !test_ASYNC_block_pause()
                || !test_ASYNC_start_job_ex()) {
            return 1;
        }
    }
    printf("PASS\n");
    return 0;
}