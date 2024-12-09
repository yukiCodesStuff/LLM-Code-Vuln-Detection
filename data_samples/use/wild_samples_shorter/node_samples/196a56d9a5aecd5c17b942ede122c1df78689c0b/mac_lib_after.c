/*
 * Copyright 2018-2022 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at

int EVP_MAC_is_a(const EVP_MAC *mac, const char *name)
{
    return mac != NULL && evp_is_a(mac->prov, mac->name_id, NULL, name);
}

int EVP_MAC_names_do_all(const EVP_MAC *mac,
                         void (*fn)(const char *name, void *data),