/*
 * Copyright 2006-2021 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at

int EVP_SIGNATURE_is_a(const EVP_SIGNATURE *signature, const char *name)
{
    return evp_is_a(signature->prov, signature->name_id, NULL, name);
}

int evp_signature_get_number(const EVP_SIGNATURE *signature)
{