/*
 * Copyright 2018-2022 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 */

#include "../testutil.h"
#include <ctype.h>
#include <openssl/provider.h>
#include <openssl/core_names.h>
#include <string.h>

int test_get_libctx(OSSL_LIB_CTX **libctx, OSSL_PROVIDER **default_null_prov,
                    const char *config_file,
    return test_get_libctx(libctx, default_null_prov,
                           test_get_argument(argn + 1), provider, module_name);
}

typedef struct {
    int major, minor, patch;
} FIPS_VERSION;

/*
 * Query the FIPS provider to determine it's version number.
 * Returns 1 if the version is retrieved correctly, 0 if the FIPS provider isn't
 * loaded and -1 on error.
 */
static int fips_provider_version(OSSL_LIB_CTX *libctx, FIPS_VERSION *vers)
{
    OSSL_PARAM params[2] = { OSSL_PARAM_END, OSSL_PARAM_END };
    OSSL_PROVIDER *fips_prov;
    char *vs;

    if (!OSSL_PROVIDER_available(libctx, "fips"))
        return 0;
    *params = OSSL_PARAM_construct_utf8_ptr(OSSL_PROV_PARAM_VERSION, &vs, 0);
    if ((fips_prov = OSSL_PROVIDER_load(libctx, "fips")) == NULL)
        return -1;
    if (!OSSL_PROVIDER_get_params(fips_prov, params)
            || sscanf(vs, "%d.%d.%d", &vers->major, &vers->minor, &vers->patch) != 3)
        goto err;
    if (!OSSL_PROVIDER_unload(fips_prov))
        return -1;
    return 1;
 err:
    OSSL_PROVIDER_unload(fips_prov);
    return -1;
}

int fips_provider_version_eq(OSSL_LIB_CTX *libctx, int major, int minor, int patch)
{
    FIPS_VERSION prov;
    int res;

    if ((res = fips_provider_version(libctx, &prov)) <= 0)
        return res == 0;
    return major == prov.major && minor == prov.minor && patch == prov.patch;
}

int fips_provider_version_ne(OSSL_LIB_CTX *libctx, int major, int minor, int patch)
{
    FIPS_VERSION prov;
    int res;

    if ((res = fips_provider_version(libctx, &prov)) <= 0)
        return res == 0;
    return major != prov.major || minor != prov.minor || patch != prov.patch;
}

int fips_provider_version_le(OSSL_LIB_CTX *libctx, int major, int minor, int patch)
{
    FIPS_VERSION prov;
    int res;

    if ((res = fips_provider_version(libctx, &prov)) <= 0)
        return res == 0;
    return prov.major < major
           || (prov.major == major
               && (prov.minor < minor
                   || (prov.minor == minor && prov.patch <= patch)));
}

int fips_provider_version_gt(OSSL_LIB_CTX *libctx, int major, int minor, int patch)
{
    FIPS_VERSION prov;
    int res;

    if ((res = fips_provider_version(libctx, &prov)) <= 0)
        return res == 0;
    return prov.major > major
           || (prov.major == major
               && (prov.minor > minor
                   || (prov.minor == minor && prov.patch > patch)));
}

int fips_provider_version_match(OSSL_LIB_CTX *libctx, const char *versions)
{
    const char *p;
    int major, minor, patch, r;
    enum {
        MODE_EQ, MODE_NE, MODE_LE, MODE_GT
    } mode;

    while (*versions != '\0') {
        for (; isspace(*versions); versions++)
            continue;
        if (*versions == '\0')
            break;
        for (p = versions; *versions != '\0' && !isspace(*versions); versions++)
            continue;
        if (*p == '!') {
            mode = MODE_NE;
            p++;
        } else if (*p == '=') {
            mode = MODE_EQ;
            p++;
        } else if (*p == '<' && p[1] == '=') {
            mode = MODE_LE;
            p += 2;
        } else if (*p == '>') {
            mode = MODE_GT;
            p++;
        } else if (isdigit(*p)) {
            mode = MODE_EQ;
        } else {
            TEST_info("Error matching FIPS version: mode %s\n", p);
            return -1;
        }
        if (sscanf(p, "%d.%d.%d", &major, &minor, &patch) != 3) {
            TEST_info("Error matching FIPS version: version %s\n", p);
            return -1;
        }
        switch (mode) {
        case MODE_EQ:
            r = fips_provider_version_eq(libctx, major, minor, patch);
            break;
        case MODE_NE:
            r = fips_provider_version_ne(libctx, major, minor, patch);
            break;
        case MODE_LE:
            r = fips_provider_version_le(libctx, major, minor, patch);
            break;
        case MODE_GT:
            r = fips_provider_version_gt(libctx, major, minor, patch);
            break;
        }
        if (r < 0) {
            TEST_info("Error matching FIPS version: internal error\n");
            return -1;
        }
        if (r == 0)
            return 0;
    }
    return 1;
}