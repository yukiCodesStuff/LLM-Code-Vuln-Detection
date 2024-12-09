/*
 * Copyright 2014-2022 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
int global_init(void);
int setup_tests(void);
void cleanup_tests(void);

/*
 * Helper functions to detect specific versions of the FIPS provider being in use.
 * Because of FIPS rules, code changes after a module has been validated are
 * difficult and because we provide a hard guarantee of ABI and behavioural
 * stability going forwards, it is a requirement to have tests be conditional
 * on specific FIPS provider versions.  Without this, bug fixes cannot be tested
 * in later releases.
 *
 * The reason for not including e.g. a less than test is to help avoid any
 * temptation to use FIPS provider version numbers that don't exist.  Until the
 * `new' provider is validated, its version isn't set in stone.  Thus a change
 * in test behaviour must depend on already validated module versions only.
 *
 * In all cases, the function returns true if:
 *      1. the FIPS provider version matches the criteria specified or
 *      2. the FIPS provider isn't being used.
 */
int fips_provider_version_eq(OSSL_LIB_CTX *libctx, int major, int minor, int patch);
int fips_provider_version_ne(OSSL_LIB_CTX *libctx, int major, int minor, int patch);
int fips_provider_version_le(OSSL_LIB_CTX *libctx, int major, int minor, int patch);
int fips_provider_version_gt(OSSL_LIB_CTX *libctx, int major, int minor, int patch);

/*
 * This function matches fips provider version with (potentially multiple)
 * <operator>maj.min.patch version strings in versions.
 * The operator can be one of = ! <= or > comparison symbols.
 * If the fips provider matches all the version comparisons (or if there is no
 * fips provider available) the function returns 1.
 * If the fips provider does not match the version comparisons, it returns 0.
 * On error the function returns -1.
 */
int fips_provider_version_match(OSSL_LIB_CTX *libctx, const char *versions);

/*
 * Used to supply test specific command line options,
 * If non optional parameters are used, then the first entry in the OPTIONS[]
 * should contain:
 */

# define PRINTF_FORMAT(a, b)
# if defined(__GNUC__) && defined(__STDC_VERSION__) \
    && !defined(__MINGW32__) && !defined(__MINGW64__) \
    && !defined(__APPLE__)
  /*
   * Because we support the 'z' modifier, which made its appearance in C99,
   * we can't use __attribute__ with pre C99 dialects.
   */