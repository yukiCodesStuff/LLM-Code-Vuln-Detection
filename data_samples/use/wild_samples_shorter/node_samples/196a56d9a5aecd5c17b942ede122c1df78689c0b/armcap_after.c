/*
 * Copyright 2011-2022 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
    }
# endif

    /*
     * Probing for ARMV7_TICK is known to produce unreliable results,
     * so we will only use the feature when the user explicitly enables
     * it with OPENSSL_armcap.
     */

    sigaction(SIGILL, &ill_oact, NULL);
    sigprocmask(SIG_SETMASK, &oset, NULL);
