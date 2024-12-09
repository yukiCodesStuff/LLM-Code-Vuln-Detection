/*
 * Copyright 2015-2022 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
    return 1;
}

/*
 * Peek ahead at 8 bytes in network order from |pkt| and store the value in
 * |*data|
 */
__owur static ossl_inline int PACKET_peek_net_8(const PACKET *pkt,
                                                uint64_t *data)
{
    if (PACKET_remaining(pkt) < 8)
        return 0;

    *data = ((uint64_t)(*pkt->curr)) << 56;
    *data |= ((uint64_t)(*(pkt->curr + 1))) << 48;
    *data |= ((uint64_t)(*(pkt->curr + 2))) << 40;
    *data |= ((uint64_t)(*(pkt->curr + 3))) << 32;
    *data |= ((uint64_t)(*(pkt->curr + 4))) << 24;
    *data |= ((uint64_t)(*(pkt->curr + 5))) << 16;
    *data |= ((uint64_t)(*(pkt->curr + 6))) << 8;
    *data |= *(pkt->curr + 7);

    return 1;
}

/* Equivalent of n2l */
/* Get 4 bytes in network order from |pkt| and store the value in |*data| */
__owur static ossl_inline int PACKET_get_net_4(PACKET *pkt, unsigned long *data)
{
    return ret;
}

/* Get 8 bytes in network order from |pkt| and store the value in |*data| */
__owur static ossl_inline int PACKET_get_net_8(PACKET *pkt, uint64_t *data)
{
    if (!PACKET_peek_net_8(pkt, data))
        return 0;

    packet_forward(pkt, 8);

    return 1;
}

/* Peek ahead at 1 byte from |pkt| and store the value in |*data| */
__owur static ossl_inline int PACKET_peek_1(const PACKET *pkt,
                                            unsigned int *data)
{
 * 1 byte will fail. Don't call this directly. Use the convenience macros below
 * instead.
 */
int WPACKET_put_bytes__(WPACKET *pkt, uint64_t val, size_t bytes);

/*
 * Convenience macros for calling WPACKET_put_bytes with different
 * lengths
    WPACKET_put_bytes__((pkt), (val), 3)
#define WPACKET_put_bytes_u32(pkt, val) \
    WPACKET_put_bytes__((pkt), (val), 4)
#define WPACKET_put_bytes_u64(pkt, val) \
    WPACKET_put_bytes__((pkt), (val), 8)

/* Set a maximum size that we will not allow the WPACKET to grow beyond */
int WPACKET_set_max_size(WPACKET *pkt, size_t maxsize);
