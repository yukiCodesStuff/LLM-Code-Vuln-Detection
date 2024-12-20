/*
 * Copyright 2018-2021 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
            goto err;
    }

    /* Use text mode for stdin */
    if (infile == NULL || strcmp(infile, "-") == 0)
        inform = FORMAT_TEXT;
    in = bio_open_default(infile, 'r', inform);
    if (in == NULL)
        goto err;
