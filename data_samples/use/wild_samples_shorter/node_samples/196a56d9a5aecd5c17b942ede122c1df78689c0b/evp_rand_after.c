/*
 * Copyright 2020-2022 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at

int EVP_RAND_is_a(const EVP_RAND *rand, const char *name)
{
    return rand != NULL && evp_is_a(rand->prov, rand->name_id, NULL, name);
}

const OSSL_PROVIDER *EVP_RAND_get0_provider(const EVP_RAND *rand)
{