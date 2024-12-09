/*
 * Copyright 2016-2022 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
    unsigned int seq, offset, len, epoch;

    BIO_clear_retry_flags(bio);
    if ((thispkt = sk_MEMPACKET_value(ctx->pkts, 0)) == NULL
        || thispkt->num != ctx->currpkt) {
        /* Probably run out of data */
        BIO_set_retry_read(bio);
        return -1;
    }
    return outl;
}

/* Take the last and penultimate packets and swap them around */
int mempacket_swap_recent(BIO *bio)
{
    MEMPACKET_TEST_CTX *ctx = BIO_get_data(bio);
    MEMPACKET *thispkt;
    int numpkts = sk_MEMPACKET_num(ctx->pkts);

    /* We need at least 2 packets to be able to swap them */
    if (numpkts <= 1)
        return 0;

    /* Get the penultimate packet */
    thispkt = sk_MEMPACKET_value(ctx->pkts, numpkts - 2);
    if (thispkt == NULL)
        return 0;

    if (sk_MEMPACKET_delete(ctx->pkts, numpkts - 2) != thispkt)
        return 0;

    /* Re-add it to the end of the list */
    thispkt->num++;
    if (sk_MEMPACKET_insert(ctx->pkts, thispkt, numpkts - 1) <= 0)
        return 0;

    /* We also have to adjust the packet number of the other packet */
    thispkt = sk_MEMPACKET_value(ctx->pkts, numpkts - 2);
    if (thispkt == NULL)
        return 0;
    thispkt->num--;

    return 1;
}

int mempacket_test_inject(BIO *bio, const char *in, int inl, int pktnum,
                          int type)
{
    MEMPACKET_TEST_CTX *ctx = BIO_get_data(bio);
        thispkt->type = type;
    }

    for (i = 0; i < sk_MEMPACKET_num(ctx->pkts); i++) {
        if (!TEST_ptr(looppkt = sk_MEMPACKET_value(ctx->pkts, i)))
            goto err;
        /* Check if we found the right place to insert this packet */
        if (looppkt->num > thispkt->num) {
            if (sk_MEMPACKET_insert(ctx->pkts, thispkt, i) == 0)
                goto err;