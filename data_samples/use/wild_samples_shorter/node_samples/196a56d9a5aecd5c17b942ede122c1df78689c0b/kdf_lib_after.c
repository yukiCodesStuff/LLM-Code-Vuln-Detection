/*
 * Copyright 2018-2022 The OpenSSL Project Authors. All Rights Reserved.
 * Copyright (c) 2018-2019, Oracle and/or its affiliates.  All rights reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy

int EVP_KDF_is_a(const EVP_KDF *kdf, const char *name)
{
    return kdf != NULL && evp_is_a(kdf->prov, kdf->name_id, NULL, name);
}

const OSSL_PROVIDER *EVP_KDF_get0_provider(const EVP_KDF *kdf)
{