/*
 * Copyright 2015-2022 The OpenSSL Project Authors. All Rights Reserved.
 * Copyright 2004-2014, Akamai Technologies. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
#ifndef OPENSSL_NO_SECURE_MEMORY
# if defined(_WIN32)
#  include <windows.h>
#  if defined(WINAPI_FAMILY_PARTITION) \
     && !WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP | WINAPI_PARTITION_SYSTEM)
/*
 * While VirtualLock is available under the app partition (e.g. UWP),
 * the headers do not define the API. Define it ourselves instead.
 */
WINBASEAPI
BOOL
WINAPI
VirtualLock(
    _In_ LPVOID lpAddress,
    _In_ SIZE_T dwSize
    );
#  endif
# endif
# include <stdlib.h>
# include <assert.h>
# if defined(OPENSSL_SYS_UNIX)