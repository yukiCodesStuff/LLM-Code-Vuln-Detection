/*
 * Copyright 2016-2022 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#include <openssl/ocsp.h>
#include "../ssl_local.h"
#include "statem_local.h"
#include "internal/cryptlib.h"

#define COOKIE_STATE_FORMAT_VERSION     1

/*
 * 2 bytes for packet length, 2 bytes for format version, 2 bytes for
 * protocol version, 2 bytes for group id, 2 bytes for cipher id, 1 byte for
 * key_share present flag, 8 bytes for timestamp, 2 bytes for the hashlen,
 * EVP_MAX_MD_SIZE for transcript hash, 1 byte for app cookie length, app cookie
 * length bytes, SHA256_DIGEST_LENGTH bytes for the HMAC of the whole thing.
 */
#define MAX_COOKIE_SIZE (2 + 2 + 2 + 2 + 2 + 1 + 8 + 2 + EVP_MAX_MD_SIZE + 1 \
                         + SSL_COOKIE_LENGTH + SHA256_DIGEST_LENGTH)

/*
 * Message header + 2 bytes for protocol version + number of random bytes +
 * + 1 byte for legacy session id length + number of bytes in legacy session id
 * + 2 bytes for ciphersuite + 1 byte for legacy compression
 * + 2 bytes for extension block length + 6 bytes for key_share extension
 * + 4 bytes for cookie extension header + the number of bytes in the cookie
 */
#define MAX_HRR_SIZE    (SSL3_HM_HEADER_LENGTH + 2 + SSL3_RANDOM_SIZE + 1 \
                         + SSL_MAX_SSL_SESSION_ID_LENGTH + 2 + 1 + 2 + 6 + 4 \
                         + MAX_COOKIE_SIZE)

/*
 * Parse the client's renegotiation binding and abort if it's not right
 */
int tls_parse_ctos_renegotiate(SSL *s, PACKET *pkt, unsigned int context,
                               X509 *x, size_t chainidx)
{
    unsigned int ilen;
    const unsigned char *data;

    /* Parse the length byte */
    if (!PACKET_get_1(pkt, &ilen)
        || !PACKET_get_bytes(pkt, &data, ilen)) {
        SSLfatal(s, SSL_AD_DECODE_ERROR, SSL_R_RENEGOTIATION_ENCODING_ERR);
        return 0;
    }

    /* Check that the extension matches */
    if (ilen != s->s3.previous_client_finished_len) {
        SSLfatal(s, SSL_AD_HANDSHAKE_FAILURE, SSL_R_RENEGOTIATION_MISMATCH);
        return 0;
    }

    if (memcmp(data, s->s3.previous_client_finished,
               s->s3.previous_client_finished_len)) {
        SSLfatal(s, SSL_AD_HANDSHAKE_FAILURE, SSL_R_RENEGOTIATION_MISMATCH);
        return 0;
    }

    s->s3.send_connection_binding = 1;

    return 1;
}
        if (!check_in_list(s, group_id, clntgroups, clnt_num_groups, 0)) {
            SSLfatal(s, SSL_AD_ILLEGAL_PARAMETER, SSL_R_BAD_KEY_SHARE);
            return 0;
        }

        /* Check if this share is for a group we can use */
        if (!check_in_list(s, group_id, srvrgroups, srvr_num_groups, 1)
                || !tls_group_allowed(s, group_id, SSL_SECOP_CURVE_SUPPORTED)
                   /*
                    * We tolerate but ignore a group id that we don't think is
                    * suitable for TLSv1.3
                    */
                || !tls_valid_group(s, group_id, TLS1_3_VERSION, TLS1_3_VERSION,
                                    0, NULL)) {
            /* Share not suitable */
            continue;
        }
{
#ifndef OPENSSL_NO_TLS1_3
    unsigned int format, version, key_share, group_id;
    EVP_MD_CTX *hctx;
    EVP_PKEY *pkey;
    PACKET cookie, raw, chhash, appcookie;
    WPACKET hrrpkt;
    const unsigned char *data, *mdin, *ciphdata;
    unsigned char hmac[SHA256_DIGEST_LENGTH];
    unsigned char hrr[MAX_HRR_SIZE];
    size_t rawlen, hmaclen, hrrlen, ciphlen;
    uint64_t tm, now;

    /* Ignore any cookie if we're not set up to verify it */
    if (s->ctx->verify_stateless_cookie_cb == NULL
            || (s->s3.flags & TLS1_FLAGS_STATELESS) == 0)
        return 1;

    if (!PACKET_as_length_prefixed_2(pkt, &cookie)) {
        SSLfatal(s, SSL_AD_DECODE_ERROR, SSL_R_LENGTH_MISMATCH);
        return 0;
    }
               != ssl_get_cipher_by_char(s, ciphdata, 0)) {
        /*
         * We chose a different cipher or group id this time around to what is
         * in the cookie. Something must have changed.
         */
        SSLfatal(s, SSL_AD_ILLEGAL_PARAMETER, SSL_R_BAD_CIPHER);
        return 0;
    }

    if (!PACKET_get_1(&cookie, &key_share)
            || !PACKET_get_net_8(&cookie, &tm)
            || !PACKET_get_length_prefixed_2(&cookie, &chhash)
            || !PACKET_get_length_prefixed_1(&cookie, &appcookie)
            || PACKET_remaining(&cookie) != SHA256_DIGEST_LENGTH) {
        SSLfatal(s, SSL_AD_DECODE_ERROR, SSL_R_LENGTH_MISMATCH);
        return 0;
    }
            || PACKET_remaining(&cookie) != SHA256_DIGEST_LENGTH) {
        SSLfatal(s, SSL_AD_DECODE_ERROR, SSL_R_LENGTH_MISMATCH);
        return 0;
    }

    /* We tolerate a cookie age of up to 10 minutes (= 60 * 10 seconds) */
    now = time(NULL);
    if (tm > now || (now - tm) > 600) {
        /* Cookie is stale. Ignore it */
        return 1;
    }
            if (sesstmp == NULL) {
                SSLfatal(s, SSL_AD_INTERNAL_ERROR, ERR_R_INTERNAL_ERROR);
                return 0;
            }
            SSL_SESSION_free(sess);
            sess = sesstmp;

            /*
             * We've just been told to use this session for this context so
             * make sure the sid_ctx matches up.
             */
            memcpy(sess->sid_ctx, s->sid_ctx, s->sid_ctx_length);
            sess->sid_ctx_length = s->sid_ctx_length;
            ext = 1;
            if (id == 0)
                s->ext.early_data_ok = 1;
            s->ext.ticket_expected = 1;
        } else {
            uint32_t ticket_age = 0, agesec, agems;
            int ret;

            /*
             * If we are using anti-replay protection then we behave as if
             * SSL_OP_NO_TICKET is set - we are caching tickets anyway so there
             * is no point in using full stateless tickets.
             */
            if ((s->options & SSL_OP_NO_TICKET) != 0
                    || (s->max_early_data > 0
                        && (s->options & SSL_OP_NO_ANTI_REPLAY) == 0))
                ret = tls_get_stateful_ticket(s, &identity, &sess);
            else
                ret = tls_decrypt_ticket(s, PACKET_data(&identity),
                                         PACKET_remaining(&identity), NULL, 0,
                                         &sess);

            if (ret == SSL_TICKET_EMPTY) {
                SSLfatal(s, SSL_AD_DECODE_ERROR, SSL_R_BAD_EXTENSION);
                return 0;
            }

            if (ret == SSL_TICKET_FATAL_ERR_MALLOC
                    || ret == SSL_TICKET_FATAL_ERR_OTHER) {
                SSLfatal(s, SSL_AD_INTERNAL_ERROR, ERR_R_INTERNAL_ERROR);
                return 0;
            }
            if (ret == SSL_TICKET_NONE || ret == SSL_TICKET_NO_DECRYPT)
                continue;

            /* Check for replay */
            if (s->max_early_data > 0
                    && (s->options & SSL_OP_NO_ANTI_REPLAY) == 0
                    && !SSL_CTX_remove_session(s->session_ctx, sess)) {
                SSL_SESSION_free(sess);
                sess = NULL;
                continue;
            }

            ticket_age = (uint32_t)ticket_agel;
            agesec = (uint32_t)(time(NULL) - sess->time);
            agems = agesec * (uint32_t)1000;
            ticket_age -= sess->ext.tick_age_add;

            /*
             * For simplicity we do our age calculations in seconds. If the
             * client does it in ms then it could appear that their ticket age
             * is longer than ours (our ticket age calculation should always be
             * slightly longer than the client's due to the network latency).
             * Therefore we add 1000ms to our age calculation to adjust for
             * rounding errors.
             */
            if (id == 0
                    && sess->timeout >= (long)agesec
                    && agems / (uint32_t)1000 == agesec
                    && ticket_age <= agems + 1000
                    && ticket_age + TICKET_AGE_ALLOWANCE >= agems + 1000) {
                /*
                 * Ticket age is within tolerance and not expired. We allow it
                 * for early data
                 */
                s->ext.early_data_ok = 1;
            }
        }
                    && !SSL_CTX_remove_session(s->session_ctx, sess)) {
                SSL_SESSION_free(sess);
                sess = NULL;
                continue;
            }

            ticket_age = (uint32_t)ticket_agel;
            agesec = (uint32_t)(time(NULL) - sess->time);
            agems = agesec * (uint32_t)1000;
            ticket_age -= sess->ext.tick_age_add;

            /*
             * For simplicity we do our age calculations in seconds. If the
             * client does it in ms then it could appear that their ticket age
             * is longer than ours (our ticket age calculation should always be
             * slightly longer than the client's due to the network latency).
             * Therefore we add 1000ms to our age calculation to adjust for
             * rounding errors.
             */
            if (id == 0
                    && sess->timeout >= (long)agesec
                    && agems / (uint32_t)1000 == agesec
                    && ticket_age <= agems + 1000
                    && ticket_age + TICKET_AGE_ALLOWANCE >= agems + 1000) {
                /*
                 * Ticket age is within tolerance and not expired. We allow it
                 * for early data
                 */
                s->ext.early_data_ok = 1;
            }
                    && ticket_age + TICKET_AGE_ALLOWANCE >= agems + 1000) {
                /*
                 * Ticket age is within tolerance and not expired. We allow it
                 * for early data
                 */
                s->ext.early_data_ok = 1;
            }
        }

        md = ssl_md(s->ctx, sess->cipher->algorithm2);
        if (md == NULL) {
            SSLfatal(s, SSL_AD_INTERNAL_ERROR, ERR_R_INTERNAL_ERROR);
            goto err;
        }
    if (s->ctx->gen_stateless_cookie_cb == NULL) {
        SSLfatal(s, SSL_AD_INTERNAL_ERROR, SSL_R_NO_COOKIE_CALLBACK_SET);
        return EXT_RETURN_FAIL;
    }

    if (!WPACKET_put_bytes_u16(pkt, TLSEXT_TYPE_cookie)
            || !WPACKET_start_sub_packet_u16(pkt)
            || !WPACKET_start_sub_packet_u16(pkt)
            || !WPACKET_get_total_written(pkt, &startlen)
            || !WPACKET_reserve_bytes(pkt, MAX_COOKIE_SIZE, &cookie)
            || !WPACKET_put_bytes_u16(pkt, COOKIE_STATE_FORMAT_VERSION)
            || !WPACKET_put_bytes_u16(pkt, TLS1_3_VERSION)
            || !WPACKET_put_bytes_u16(pkt, s->s3.group_id)
            || !s->method->put_cipher_by_char(s->s3.tmp.new_cipher, pkt,
                                              &ciphlen)
               /* Is there a key_share extension present in this HRR? */
            || !WPACKET_put_bytes_u8(pkt, s->s3.peer_tmp == NULL)
            || !WPACKET_put_bytes_u64(pkt, time(NULL))
            || !WPACKET_start_sub_packet_u16(pkt)
            || !WPACKET_reserve_bytes(pkt, EVP_MAX_MD_SIZE, &hashval1)) {
        SSLfatal(s, SSL_AD_INTERNAL_ERROR, ERR_R_INTERNAL_ERROR);
        return EXT_RETURN_FAIL;
    }