/*
 * Copyright 2018-2022 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#include "../testutil.h"
#include <ctype.h>
#include <openssl/provider.h>
#include <openssl/core_names.h>
#include <string.h>

int test_get_libctx(OSSL_LIB_CTX **libctx, OSSL_PROVIDER **default_null_prov,
                    const char *config_file,
                    OSSL_PROVIDER **provider, const char *module_name)
{
    OSSL_LIB_CTX *new_libctx = NULL;

    if (libctx != NULL) {
        if ((new_libctx = *libctx = OSSL_LIB_CTX_new()) == NULL) {
            opt_printf_stderr("Failed to create libctx\n");
            goto err;
        }
    }

    if (default_null_prov != NULL
        && (*default_null_prov = OSSL_PROVIDER_load(NULL, "null")) == NULL) {
        opt_printf_stderr("Failed to load null provider into default libctx\n");
        goto err;
    }

    if (config_file != NULL
            && !OSSL_LIB_CTX_load_config(new_libctx, config_file)) {
        opt_printf_stderr("Error loading config from file %s\n", config_file);
        goto err;
    }

    if (module_name != NULL
            && (*provider = OSSL_PROVIDER_load(new_libctx, module_name)) == NULL) {
        opt_printf_stderr("Failed to load provider %s\n", module_name);
        goto err;
    }
    return 1;

 err:
    ERR_print_errors_fp(stderr);
    return 0;
}
/*
 * Copyright 2018-2022 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#include "../testutil.h"
#include <ctype.h>
#include <openssl/provider.h>
#include <openssl/core_names.h>
#include <string.h>

int test_get_libctx(OSSL_LIB_CTX **libctx, OSSL_PROVIDER **default_null_prov,
                    const char *config_file,
                    OSSL_PROVIDER **provider, const char *module_name)
{
    OSSL_LIB_CTX *new_libctx = NULL;

    if (libctx != NULL) {
        if ((new_libctx = *libctx = OSSL_LIB_CTX_new()) == NULL) {
            opt_printf_stderr("Failed to create libctx\n");
            goto err;
        }
    }

    if (default_null_prov != NULL
        && (*default_null_prov = OSSL_PROVIDER_load(NULL, "null")) == NULL) {
        opt_printf_stderr("Failed to load null provider into default libctx\n");
        goto err;
    }

    if (config_file != NULL
            && !OSSL_LIB_CTX_load_config(new_libctx, config_file)) {
        opt_printf_stderr("Error loading config from file %s\n", config_file);
        goto err;
    }

    if (module_name != NULL
            && (*provider = OSSL_PROVIDER_load(new_libctx, module_name)) == NULL) {
        opt_printf_stderr("Failed to load provider %s\n", module_name);
        goto err;
    }
    return 1;

 err:
    ERR_print_errors_fp(stderr);
    return 0;
}
    if (!TEST_ptr(module_name = test_get_argument(argn))) {
        TEST_error("usage: <prog> %s", usage);
        return 0;
    }
    if (strcmp(module_name, "none") == 0)
        return 1;
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