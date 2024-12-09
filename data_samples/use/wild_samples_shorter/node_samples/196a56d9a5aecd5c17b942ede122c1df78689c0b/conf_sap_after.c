/*
 * Copyright 2002-2022 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
int ossl_config_int(const OPENSSL_INIT_SETTINGS *settings)
{
    int ret = 0;
#if defined(OPENSSL_INIT_DEBUG) || !defined(OPENSSL_SYS_UEFI)
    const char *filename;
    const char *appname;
    unsigned long flags;
#endif

    if (openssl_configured)
        return 1;

#if defined(OPENSSL_INIT_DEBUG) || !defined(OPENSSL_SYS_UEFI)
    filename = settings ? settings->filename : NULL;
    appname = settings ? settings->appname : NULL;
    flags = settings ? settings->flags : DEFAULT_CONF_MFLAGS;
#endif

#ifdef OPENSSL_INIT_DEBUG
    fprintf(stderr, "OPENSSL_INIT: ossl_config_int(%s, %s, %lu)\n",
            filename, appname, flags);