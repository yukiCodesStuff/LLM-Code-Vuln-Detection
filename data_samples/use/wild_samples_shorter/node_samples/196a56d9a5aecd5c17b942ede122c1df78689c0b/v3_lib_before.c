/*
 * Copyright 1999-2020 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
        }
        /* If delete, just delete it */
        if (ext_op == X509V3_ADD_DELETE) {
            if (!sk_X509_EXTENSION_delete(*x, extidx))
                return -1;
            return 1;
        }
    } else {
        /*