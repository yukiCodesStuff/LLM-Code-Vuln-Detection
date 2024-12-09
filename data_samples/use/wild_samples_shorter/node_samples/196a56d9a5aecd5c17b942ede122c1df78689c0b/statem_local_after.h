/*
 * Copyright 2015-2022 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
/* Dummy message type */
#define SSL3_MT_DUMMY   -1

/* Invalid extension ID for non-supported extensions */
#define TLSEXT_TYPE_invalid            0x10000
#define TLSEXT_TYPE_out_of_range       0x10001
unsigned int ossl_get_extension_type(size_t idx);

extern const unsigned char hrrrandom[];

/* Message processing return codes */
typedef enum {