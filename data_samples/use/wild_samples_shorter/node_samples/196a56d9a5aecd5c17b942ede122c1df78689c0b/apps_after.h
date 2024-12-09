/*
 * Copyright 1995-2022 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
# include "e_os.h" /* struct timeval for DTLS */
# include "internal/nelem.h"
# include "internal/sockets.h" /* for openssl_fdset() */
# include "internal/cryptlib.h"  /* ossl_assert() */
# include <assert.h>

# include <stdarg.h>
# include <sys/types.h>

void app_bail_out(char *fmt, ...);
void *app_malloc(size_t sz, const char *what);

/* load_serial, save_serial, and rotate_serial are also used for CRL numbers */
BIGNUM *load_serial(const char *serialfile, int *exists, int create,
                    ASN1_INTEGER **retai);
int save_serial(const char *serialfile, const char *suffix,
                const BIGNUM *serial, ASN1_INTEGER **retai);
int rotate_serial(const char *serialfile, const char *new_suffix,
                  const char *old_suffix);
int rand_serial(BIGNUM *b, ASN1_INTEGER *ai);

CA_DB *load_index(const char *dbfile, DB_ATTR *dbattr);
int index_index(CA_DB *db);
int save_index(const char *dbfile, const char *suffix, CA_DB *db);
int rotate_index(const char *dbfile, const char *new_suffix,