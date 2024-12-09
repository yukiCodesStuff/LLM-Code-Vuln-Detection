/*
 * Copyright 2016-2021 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
/*
 * Get and cleanup random seed material.
 */
size_t ossl_rand_get_entropy(ossl_unused OSSL_CORE_HANDLE *handle,
                             unsigned char **pout, int entropy,
                             size_t min_len, size_t max_len);
void ossl_rand_cleanup_entropy(ossl_unused OSSL_CORE_HANDLE *handle,
                               unsigned char *buf, size_t len);
size_t ossl_rand_get_nonce(ossl_unused OSSL_CORE_HANDLE *handle,
                           unsigned char **pout, size_t min_len, size_t max_len,
                           const void *salt, size_t salt_len);
void ossl_rand_cleanup_nonce(ossl_unused OSSL_CORE_HANDLE *handle,
                             unsigned char *buf, size_t len);

/*
 * Get seeding material from the operating system sources.