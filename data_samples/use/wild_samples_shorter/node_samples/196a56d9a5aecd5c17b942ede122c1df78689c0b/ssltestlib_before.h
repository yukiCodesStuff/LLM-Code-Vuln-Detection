/*
 * Copyright 2016-2020 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
#define MEMPACKET_CTRL_GET_DROP_REC         (3 << 15)
#define MEMPACKET_CTRL_SET_DUPLICATE_REC    (4 << 15)

int mempacket_test_inject(BIO *bio, const char *in, int inl, int pktnum,
                          int type);

typedef struct mempacket_st MEMPACKET;