/*
 * Copyright 1995-2021 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
# define ERR_R_DSA_LIB          (ERR_LIB_DSA/* 10 */ | ERR_RFLAG_COMMON)
# define ERR_R_X509_LIB         (ERR_LIB_X509/* 11 */ | ERR_RFLAG_COMMON)
# define ERR_R_ASN1_LIB         (ERR_LIB_ASN1/* 13 */ | ERR_RFLAG_COMMON)
# define ERR_R_CRYPTO_LIB       (ERR_LIB_CRYPTO/* 15 */ | ERR_RFLAG_COMMON)
# define ERR_R_EC_LIB           (ERR_LIB_EC/* 16 */ | ERR_RFLAG_COMMON)
# define ERR_R_BIO_LIB          (ERR_LIB_BIO/* 32 */ | ERR_RFLAG_COMMON)
# define ERR_R_PKCS7_LIB        (ERR_LIB_PKCS7/* 33 */ | ERR_RFLAG_COMMON)
# define ERR_R_X509V3_LIB       (ERR_LIB_X509V3/* 34 */ | ERR_RFLAG_COMMON)
# define ERR_R_ENGINE_LIB       (ERR_LIB_ENGINE/* 38 */ | ERR_RFLAG_COMMON)
# define ERR_R_UI_LIB           (ERR_LIB_UI/* 40 */ | ERR_RFLAG_COMMON)
# define ERR_R_ECDSA_LIB        (ERR_LIB_ECDSA/* 42 */ | ERR_RFLAG_COMMON)
# define ERR_R_OSSL_STORE_LIB   (ERR_LIB_OSSL_STORE/* 44 */ | ERR_RFLAG_COMMON)
# define ERR_R_OSSL_DECODER_LIB (ERR_LIB_OSSL_DECODER/* 60 */ | ERR_RFLAG_COMMON)

/* Other common error codes, range 256..2^ERR_RFLAGS_OFFSET-1 */
# define ERR_R_FATAL                             (ERR_RFLAG_FATAL|ERR_RFLAG_COMMON)