/*
 * Copyright 1995-2022 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
    }

    tmpoid->nid = OBJ_new_nid(1);
    if (tmpoid->nid == NID_undef)
        goto err;

    tmpoid->sn = (char *)sn;
    tmpoid->ln = (char *)ln;

    ok = OBJ_add_object(tmpoid);