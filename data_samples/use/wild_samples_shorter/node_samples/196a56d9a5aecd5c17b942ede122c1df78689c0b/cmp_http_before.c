/*
 * Copyright 2007-2021 The OpenSSL Project Authors. All Rights Reserved.
 * Copyright Nokia 2007-2019
 * Copyright Siemens AG 2015-2019
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
static int keep_alive(int keep_alive, int body_type)
{
    if (keep_alive != 0
        /* Ask for persistent connection only if may need more round trips */
            && body_type != OSSL_CMP_PKIBODY_IR
            && body_type != OSSL_CMP_PKIBODY_CR
            && body_type != OSSL_CMP_PKIBODY_P10CR
            && body_type != OSSL_CMP_PKIBODY_KUR