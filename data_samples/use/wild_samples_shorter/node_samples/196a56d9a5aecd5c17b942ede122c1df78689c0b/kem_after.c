/*
 * Copyright 2020-2022 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at

int EVP_KEM_is_a(const EVP_KEM *kem, const char *name)
{
    return kem != NULL && evp_is_a(kem->prov, kem->name_id, NULL, name);
}

int evp_kem_get_number(const EVP_KEM *kem)
{