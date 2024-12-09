/*
 * Copyright 2014-2021 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
int global_init(void);
int setup_tests(void);
void cleanup_tests(void);
/*
 * Used to supply test specific command line options,
 * If non optional parameters are used, then the first entry in the OPTIONS[]
 * should contain:
 */

# define PRINTF_FORMAT(a, b)
# if defined(__GNUC__) && defined(__STDC_VERSION__)
  /*
   * Because we support the 'z' modifier, which made its appearance in C99,
   * we can't use __attribute__ with pre C99 dialects.
   */