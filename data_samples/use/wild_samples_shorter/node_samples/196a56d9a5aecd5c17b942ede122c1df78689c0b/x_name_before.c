/*
 * Copyright 1995-2021 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
int X509_NAME_print(BIO *bp, const X509_NAME *name, int obase)
{
    char *s, *c, *b;
    int l, i;

    l = 80 - 2 - obase;

    b = X509_NAME_oneline(name, NULL, 0);
    if (b == NULL)
        return 0;
                if (BIO_write(bp, ", ", 2) != 2)
                    goto err;
            }
            l--;
        }
        if (*s == '\0')
            break;
        s++;
        l--;
    }

    OPENSSL_free(b);
    return 1;