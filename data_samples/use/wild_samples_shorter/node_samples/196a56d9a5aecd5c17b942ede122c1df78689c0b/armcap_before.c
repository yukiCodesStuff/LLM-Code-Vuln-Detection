/*
 * Copyright 2011-2021 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
    }
# endif

    /* Things that getauxval didn't tell us */
    if (sigsetjmp(ill_jmp, 1) == 0) {
        _armv7_tick();
        OPENSSL_armcap_P |= ARMV7_TICK;
    }

    sigaction(SIGILL, &ill_oact, NULL);
    sigprocmask(SIG_SETMASK, &oset, NULL);
