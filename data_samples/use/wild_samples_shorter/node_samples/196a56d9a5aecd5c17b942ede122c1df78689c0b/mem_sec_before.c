/*
 * Copyright 2015-2021 The OpenSSL Project Authors. All Rights Reserved.
 * Copyright 2004-2014, Akamai Technologies. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
#ifndef OPENSSL_NO_SECURE_MEMORY
# if defined(_WIN32)
#  include <windows.h>
# endif
# include <stdlib.h>
# include <assert.h>
# if defined(OPENSSL_SYS_UNIX)