/*
 * Copyright 2011-2021 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <signal.h>
#include <openssl/crypto.h>
#ifdef __APPLE__
#include <sys/sysctl.h>
#endif
#include "internal/cryptlib.h"

#include "arm_arch.h"

unsigned int OPENSSL_armcap_P = 0;
unsigned int OPENSSL_arm_midr = 0;
unsigned int OPENSSL_armv8_rsa_neonized = 0;

#if __ARM_MAX_ARCH__<7
void OPENSSL_cpuid_setup(void)
{
}
        if (sigsetjmp(ill_jmp, 1) == 0) {
            _armv8_sha512_probe();
            OPENSSL_armcap_P |= ARMV8_SHA512;
        }
#  endif
    }
# endif

    /* Things that getauxval didn't tell us */
    if (sigsetjmp(ill_jmp, 1) == 0) {
        _armv7_tick();
        OPENSSL_armcap_P |= ARMV7_TICK;
    }