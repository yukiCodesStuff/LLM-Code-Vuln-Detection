/*
 * Copyright 1995-2022 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at

# ifdef USE_BCRYPTGENRANDOM
#  include <bcrypt.h>
#  ifdef _MSC_VER
#   pragma comment(lib, "bcrypt.lib")
#  endif
#  ifndef STATUS_SUCCESS
#   define STATUS_SUCCESS ((NTSTATUS)0x00000000L)
#  endif
# else