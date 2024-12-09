/*
 * Copyright 2016-2021 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
    unsigned int seq, offset, len, epoch;

    BIO_clear_retry_flags(bio);
    thispkt = sk_MEMPACKET_value(ctx->pkts, 0);
    if (thispkt == NULL || thispkt->num != ctx->currpkt) {
        /* Probably run out of data */
        BIO_set_retry_read(bio);
        return -1;
    }
    return outl;
}

int mempacket_test_inject(BIO *bio, const char *in, int inl, int pktnum,
                          int type)
{
    MEMPACKET_TEST_CTX *ctx = BIO_get_data(bio);
        thispkt->type = type;
    }

    for(i = 0; (looppkt = sk_MEMPACKET_value(ctx->pkts, i)) != NULL; i++) {
        /* Check if we found the right place to insert this packet */
        if (looppkt->num > thispkt->num) {
            if (sk_MEMPACKET_insert(ctx->pkts, thispkt, i) == 0)
                goto err;