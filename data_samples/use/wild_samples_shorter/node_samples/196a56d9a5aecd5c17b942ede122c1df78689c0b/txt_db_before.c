/*
 * Copyright 1995-2018 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
{
    TXT_DB *ret = NULL;
    int esc = 0;
    long ln = 0;
    int i, add, n;
    int size = BUFSIZE;
    int offset = 0;
    char *p, *f;
        }
        buf->data[offset] = '\0';
        BIO_gets(in, &(buf->data[offset]), size - offset);
        ln++;
        if (buf->data[offset] == '\0')
            break;
        if ((offset == 0) && (buf->data[0] == '#'))
            continue;