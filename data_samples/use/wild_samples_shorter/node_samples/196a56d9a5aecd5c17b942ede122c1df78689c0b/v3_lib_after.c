/*
 * Copyright 1999-2022 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
        }
        /* If delete, just delete it */
        if (ext_op == X509V3_ADD_DELETE) {
            extmp = sk_X509_EXTENSION_delete(*x, extidx);
            if (extmp == NULL)
                return -1;
            X509_EXTENSION_free(extmp);
            return 1;
        }
    } else {
        /*