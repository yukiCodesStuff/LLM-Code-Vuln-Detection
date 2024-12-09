/*
 * Copyright 2015-2021 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
}

/* Store the |value| of length |len| at location |data| */
static int put_value(unsigned char *data, size_t value, size_t len)
{
    if (data == NULL)
        return 1;

    return WPACKET_start_sub_packet_len__(pkt, 0);
}

int WPACKET_put_bytes__(WPACKET *pkt, unsigned int val, size_t size)
{
    unsigned char *data;

    /* Internal API, so should not fail */
    if (!ossl_assert(size <= sizeof(unsigned int))
            || !WPACKET_allocate_bytes(pkt, size, &data)
            || !put_value(data, val, size))
        return 0;
