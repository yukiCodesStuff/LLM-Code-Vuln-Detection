/*
 * Copyright 1995-2021 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#include <stdio.h>
#include <limits.h>
#include <errno.h>
#include "../ssl_local.h"
#include <openssl/evp.h>
#include <openssl/buffer.h>
#include <openssl/rand.h>
#include "record_local.h"
#include "internal/packet.h"

#if     defined(OPENSSL_SMALL_FOOTPRINT) || \
        !(      defined(AES_ASM) &&     ( \
                defined(__x86_64)       || defined(__x86_64__)  || \
                defined(_M_AMD64)       || defined(_M_X64)      ) \
        )
# undef EVP_CIPH_FLAG_TLS1_1_MULTIBLOCK
# define EVP_CIPH_FLAG_TLS1_1_MULTIBLOCK 0
#endif

void RECORD_LAYER_init(RECORD_LAYER *rl, SSL *s)
{
    rl->s = s;
    RECORD_LAYER_set_first_record(&s->rlayer);
    SSL3_RECORD_clear(rl->rrec, SSL_MAX_PIPELINES);
}
{
    size_t i, num = 0;

    if (s->rlayer.rstate == SSL_ST_READ_BODY)
        return 0;

    for (i = 0; i < RECORD_LAYER_get_numrpipes(&s->rlayer); i++) {
        if (SSL3_RECORD_get_type(&s->rlayer.rrec[i])
            != SSL3_RT_APPLICATION_DATA)
            return 0;
        num += SSL3_RECORD_get_length(&s->rlayer.rrec[i]);
    }
        if (mode == EVP_CIPH_CBC_MODE) {
            eivlen = EVP_CIPHER_CTX_get_iv_length(s->enc_write_ctx);
            if (eivlen <= 1)
                eivlen = 0;
        } else if (mode == EVP_CIPH_GCM_MODE) {
            /* Need explicit part of IV for GCM mode */
            eivlen = EVP_GCM_TLS_EXPLICIT_IV_LEN;
        } else if (mode == EVP_CIPH_CCM_MODE) {
            eivlen = EVP_CCM_TLS_EXPLICIT_IV_LEN;
        }
    }

 wpacket_init_complete:

    totlen = 0;
    /* Clear our SSL3_RECORD structures */
    memset(wr, 0, sizeof(wr));
    for (j = 0; j < numpipes; j++) {
        unsigned int version = (s->version == TLS1_3_VERSION) ? TLS1_2_VERSION
                                                              : s->version;
        unsigned char *compressdata = NULL;
        size_t maxcomplen;
        unsigned int rectype;

        thispkt = &pkt[j];
        thiswr = &wr[j];

        /*
         * In TLSv1.3, once encrypting, we always use application data for the
         * record type
         */
        if (SSL_TREAT_AS_TLS13(s)
                && s->enc_write_ctx != NULL
                && (s->statem.enc_write_state != ENC_WRITE_STATE_WRITE_PLAIN_ALERTS
                    || type != SSL3_RT_ALERT))
            rectype = SSL3_RT_APPLICATION_DATA;
        else
            rectype = type;
        SSL3_RECORD_set_type(thiswr, rectype);

        /*
         * Some servers hang if initial client hello is larger than 256 bytes
         * and record version number > TLS 1.0
         */
        if (SSL_get_state(s) == TLS_ST_CW_CLNT_HELLO
                && !s->renegotiate
                && TLS1_get_version(s) > TLS1_VERSION
                && s->hello_retry_request == SSL_HRR_NONE)
            version = TLS1_VERSION;
        SSL3_RECORD_set_rec_version(thiswr, version);

        maxcomplen = pipelens[j];
        if (s->compress != NULL)
            maxcomplen += SSL3_RT_MAX_COMPRESSED_OVERHEAD;

        /*
         * When using offload kernel will write the header.
         * Otherwise write the header now
         */
        if (!BIO_get_ktls_send(s->wbio)
                && (!WPACKET_put_bytes_u8(thispkt, rectype)
                || !WPACKET_put_bytes_u16(thispkt, version)
                || !WPACKET_start_sub_packet_u16(thispkt)
                || (eivlen > 0
                    && !WPACKET_allocate_bytes(thispkt, eivlen, NULL))
                || (maxcomplen > 0
                    && !WPACKET_reserve_bytes(thispkt, maxcomplen,
                                              &compressdata)))) {
            SSLfatal(s, SSL_AD_INTERNAL_ERROR, ERR_R_INTERNAL_ERROR);
            goto err;
        }

        /* lets setup the record stuff. */
        SSL3_RECORD_set_data(thiswr, compressdata);
        SSL3_RECORD_set_length(thiswr, pipelens[j]);
        SSL3_RECORD_set_input(thiswr, (unsigned char *)&buf[totlen]);
        totlen += pipelens[j];

        /*
         * we now 'read' from thiswr->input, thiswr->length bytes into
         * thiswr->data
         */

        /* first we compress */
        if (s->compress != NULL) {
            if (!ssl3_do_compress(s, thiswr)
                    || !WPACKET_allocate_bytes(thispkt, thiswr->length, NULL)) {
                SSLfatal(s, SSL_AD_INTERNAL_ERROR, SSL_R_COMPRESSION_FAILURE);
                goto err;
            }
        } else {
            if (BIO_get_ktls_send(s->wbio)) {
                SSL3_RECORD_reset_data(&wr[j]);
            } else {
                if (!WPACKET_memcpy(thispkt, thiswr->input, thiswr->length)) {
                    SSLfatal(s, SSL_AD_INTERNAL_ERROR, ERR_R_INTERNAL_ERROR);
                    goto err;
                }
                SSL3_RECORD_reset_input(&wr[j]);
            }
        }

        if (SSL_TREAT_AS_TLS13(s)
                && !BIO_get_ktls_send(s->wbio)
                && s->enc_write_ctx != NULL
                && (s->statem.enc_write_state != ENC_WRITE_STATE_WRITE_PLAIN_ALERTS
                    || type != SSL3_RT_ALERT)) {
            size_t rlen, max_send_fragment;

            if (!WPACKET_put_bytes_u8(thispkt, type)) {
                SSLfatal(s, SSL_AD_INTERNAL_ERROR, ERR_R_INTERNAL_ERROR);
                goto err;
            }
            SSL3_RECORD_add_length(thiswr, 1);

            /* Add TLS1.3 padding */
            max_send_fragment = ssl_get_max_send_fragment(s);
            rlen = SSL3_RECORD_get_length(thiswr);
            if (rlen < max_send_fragment) {
                size_t padding = 0;
                size_t max_padding = max_send_fragment - rlen;
                if (s->record_padding_cb != NULL) {
                    padding = s->record_padding_cb(s, type, rlen, s->record_padding_arg);
                } else if (s->block_padding > 0) {
                    size_t mask = s->block_padding - 1;
                    size_t remainder;

                    /* optimize for power of 2 */
                    if ((s->block_padding & mask) == 0)
                        remainder = rlen & mask;
                    else
                        remainder = rlen % s->block_padding;
                    /* don't want to add a block of padding if we don't have to */
                    if (remainder == 0)
                        padding = 0;
                    else
                        padding = s->block_padding - remainder;
                }
                if (padding > 0) {
                    /* do not allow the record to exceed max plaintext length */
                    if (padding > max_padding)
                        padding = max_padding;
                    if (!WPACKET_memset(thispkt, 0, padding)) {
                        SSLfatal(s, SSL_AD_INTERNAL_ERROR,
                                 ERR_R_INTERNAL_ERROR);
                        goto err;
                    }
                    SSL3_RECORD_add_length(thiswr, padding);
                }
            }
        }

        /*
         * we should still have the output to thiswr->data and the input from
         * wr->input. Length should be thiswr->length. thiswr->data still points
         * in the wb->buf
         */

        if (!BIO_get_ktls_send(s->wbio) && !SSL_WRITE_ETM(s) && mac_size != 0) {
            unsigned char *mac;

            if (!WPACKET_allocate_bytes(thispkt, mac_size, &mac)
                    || !s->method->ssl3_enc->mac(s, thiswr, mac, 1)) {
                SSLfatal(s, SSL_AD_INTERNAL_ERROR, ERR_R_INTERNAL_ERROR);
                goto err;
            }
        }

        /*
         * Reserve some bytes for any growth that may occur during encryption.
         * This will be at most one cipher block or the tag length if using
         * AEAD. SSL_RT_MAX_CIPHER_BLOCK_SIZE covers either case.
         */
        if (!BIO_get_ktls_send(s->wbio)) {
            if (!WPACKET_reserve_bytes(thispkt,
                                        SSL_RT_MAX_CIPHER_BLOCK_SIZE,
                                        NULL)
                /*
                 * We also need next the amount of bytes written to this
                 * sub-packet
                 */
                || !WPACKET_get_length(thispkt, &len)) {
            SSLfatal(s, SSL_AD_INTERNAL_ERROR, ERR_R_INTERNAL_ERROR);
            goto err;
            }

            /* Get a pointer to the start of this record excluding header */
            recordstart = WPACKET_get_curr(thispkt) - len;
            SSL3_RECORD_set_data(thiswr, recordstart);
            SSL3_RECORD_reset_input(thiswr);
            SSL3_RECORD_set_length(thiswr, len);
        }
    }

    if (s->statem.enc_write_state == ENC_WRITE_STATE_WRITE_PLAIN_ALERTS) {
        /*
         * We haven't actually negotiated the version yet, but we're trying to
         * send early data - so we need to use the tls13enc function.
         */
        if (tls13_enc(s, wr, numpipes, 1, NULL, mac_size) < 1) {
            if (!ossl_statem_in_error(s)) {
                SSLfatal(s, SSL_AD_INTERNAL_ERROR, ERR_R_INTERNAL_ERROR);
            }
            goto err;
        }
    } else {
        if (!BIO_get_ktls_send(s->wbio)) {
            if (s->method->ssl3_enc->enc(s, wr, numpipes, 1, NULL,
                                         mac_size) < 1) {
                if (!ossl_statem_in_error(s)) {
                    SSLfatal(s, SSL_AD_INTERNAL_ERROR, ERR_R_INTERNAL_ERROR);
                }
                goto err;
            }
        }
    }

    for (j = 0; j < numpipes; j++) {
        size_t origlen;

        thispkt = &pkt[j];
        thiswr = &wr[j];

        if (BIO_get_ktls_send(s->wbio))
            goto mac_done;

        /* Allocate bytes for the encryption overhead */
        if (!WPACKET_get_length(thispkt, &origlen)
                   /* Encryption should never shrink the data! */
                || origlen > thiswr->length
                || (thiswr->length > origlen
                    && !WPACKET_allocate_bytes(thispkt,
                                               thiswr->length - origlen,
                                               NULL))) {
            SSLfatal(s, SSL_AD_INTERNAL_ERROR, ERR_R_INTERNAL_ERROR);
            goto err;
        }
        if (SSL_WRITE_ETM(s) && mac_size != 0) {
            unsigned char *mac;

            if (!WPACKET_allocate_bytes(thispkt, mac_size, &mac)
                    || !s->method->ssl3_enc->mac(s, thiswr, mac, 1)) {
                SSLfatal(s, SSL_AD_INTERNAL_ERROR, ERR_R_INTERNAL_ERROR);
                goto err;
            }
            SSL3_RECORD_add_length(thiswr, mac_size);
        }

        if (!WPACKET_get_length(thispkt, &len)
                || !WPACKET_close(thispkt)) {
            SSLfatal(s, SSL_AD_INTERNAL_ERROR, ERR_R_INTERNAL_ERROR);
            goto err;
        }

        if (s->msg_callback) {
            recordstart = WPACKET_get_curr(thispkt) - len
                          - SSL3_RT_HEADER_LENGTH;
            s->msg_callback(1, thiswr->rec_version, SSL3_RT_HEADER, recordstart,
                            SSL3_RT_HEADER_LENGTH, s,
                            s->msg_callback_arg);

            if (SSL_TREAT_AS_TLS13(s) && s->enc_write_ctx != NULL) {
                unsigned char ctype = type;

                s->msg_callback(1, thiswr->rec_version, SSL3_RT_INNER_CONTENT_TYPE,
                                &ctype, 1, s, s->msg_callback_arg);
            }
        }

        if (!WPACKET_finish(thispkt)) {
            SSLfatal(s, SSL_AD_INTERNAL_ERROR, ERR_R_INTERNAL_ERROR);
            goto err;
        }

        /* header is added by the kernel when using offload */
        SSL3_RECORD_add_length(&wr[j], SSL3_RT_HEADER_LENGTH);

        if (create_empty_fragment) {
            /*
             * we are in a recursive call; just return the length, don't write
             * out anything here
             */
            if (j > 0) {
                /* We should never be pipelining an empty fragment!! */
                SSLfatal(s, SSL_AD_INTERNAL_ERROR, ERR_R_INTERNAL_ERROR);
                goto err;
            }
            *written = SSL3_RECORD_get_length(thiswr);
            return 1;
        }

 mac_done:
        /*
         * we should now have thiswr->data pointing to the encrypted data, which
         * is thiswr->length long
         */
        SSL3_RECORD_set_type(thiswr, type); /* not needed but helps for
                                             * debugging */

        /* now let's set up wb */
        SSL3_BUFFER_set_left(&s->rlayer.wbuf[j],
                             prefix_len + SSL3_RECORD_get_length(thiswr));
    }

    /*
     * memorize arguments so that ssl3_write_pending can detect bad write
     * retries later
     */
    s->rlayer.wpend_tot = totlen;
    s->rlayer.wpend_buf = buf;
    s->rlayer.wpend_type = type;
    s->rlayer.wpend_ret = totlen;

    /* we now just need to write the buffer */
    return ssl3_write_pending(s, type, buf, totlen, written);
 err:
    for (j = 0; j < wpinited; j++)
        WPACKET_cleanup(&pkt[j]);
    return -1;
}

/* if s->s3.wbuf.left != 0, we need to call this
 *
 * Return values are as per SSL_write()
 */
int ssl3_write_pending(SSL *s, int type, const unsigned char *buf, size_t len,
                       size_t *written)
{
    int i;
    SSL3_BUFFER *wb = s->rlayer.wbuf;
    size_t currbuf = 0;
    size_t tmpwrit = 0;

    if ((s->rlayer.wpend_tot > len)
        || (!(s->mode & SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER)
            && (s->rlayer.wpend_buf != buf))
        || (s->rlayer.wpend_type != type)) {
        SSLfatal(s, SSL_AD_INTERNAL_ERROR, SSL_R_BAD_WRITE_RETRY);
        return -1;
    }

    for (;;) {
        /* Loop until we find a buffer we haven't written out yet */
        if (SSL3_BUFFER_get_left(&wb[currbuf]) == 0
            && currbuf < s->rlayer.numwpipes - 1) {
            currbuf++;
            continue;
        }
        clear_sys_error();
        if (s->wbio != NULL) {
            s->rwstate = SSL_WRITING;

            /*
             * To prevent coalescing of control and data messages,
             * such as in buffer_write, we flush the BIO
             */
            if (BIO_get_ktls_send(s->wbio) && type != SSL3_RT_APPLICATION_DATA) {
                i = BIO_flush(s->wbio);
                if (i <= 0)
                    return i;
                BIO_set_ktls_ctrl_msg(s->wbio, type);
            }
            i = BIO_write(s->wbio, (char *)
                          &(SSL3_BUFFER_get_buf(&wb[currbuf])
                            [SSL3_BUFFER_get_offset(&wb[currbuf])]),
                          (unsigned int)SSL3_BUFFER_get_left(&wb[currbuf]));
            if (i >= 0)
                tmpwrit = i;
        } else {
            SSLfatal(s, SSL_AD_INTERNAL_ERROR, SSL_R_BIO_NOT_SET);
            i = -1;
        }

        /*
         * When an empty fragment is sent on a connection using KTLS,
         * it is sent as a write of zero bytes.  If this zero byte
         * write succeeds, i will be 0 rather than a non-zero value.
         * Treat i == 0 as success rather than an error for zero byte
         * writes to permit this case.
         */
        if (i >= 0 && tmpwrit == SSL3_BUFFER_get_left(&wb[currbuf])) {
            SSL3_BUFFER_set_left(&wb[currbuf], 0);
            SSL3_BUFFER_add_offset(&wb[currbuf], tmpwrit);
            if (currbuf + 1 < s->rlayer.numwpipes)
                continue;
            s->rwstate = SSL_NOTHING;
            *written = s->rlayer.wpend_ret;
            return 1;
        } else if (i <= 0) {
            if (SSL_IS_DTLS(s)) {
                /*
                 * For DTLS, just drop it. That's kind of the whole point in
                 * using a datagram service
                 */
                SSL3_BUFFER_set_left(&wb[currbuf], 0);
            }
            return i;
        }
        SSL3_BUFFER_add_offset(&wb[currbuf], tmpwrit);
        SSL3_BUFFER_sub_left(&wb[currbuf], tmpwrit);
    }
}

/*-
 * Return up to 'len' payload bytes received in 'type' records.
 * 'type' is one of the following:
 *
 *   -  SSL3_RT_HANDSHAKE (when ssl3_get_message calls us)
 *   -  SSL3_RT_APPLICATION_DATA (when ssl3_read calls us)
 *   -  0 (during a shutdown, no data has to be returned)
 *
 * If we don't have stored data to work from, read a SSL/TLS record first
 * (possibly multiple records if we still don't have anything to return).
 *
 * This function must handle any surprises the peer may have for us, such as
 * Alert records (e.g. close_notify) or renegotiation requests. ChangeCipherSpec
 * messages are treated as if they were handshake messages *if* the |recvd_type|
 * argument is non NULL.
 * Also if record payloads contain fragments too small to process, we store
 * them until there is enough for the respective protocol (the record protocol
 * may use arbitrary fragmentation and even interleaving):
 *     Change cipher spec protocol
 *             just 1 byte needed, no need for keeping anything stored
 *     Alert protocol
 *             2 bytes needed (AlertLevel, AlertDescription)
 *     Handshake protocol
 *             4 bytes needed (HandshakeType, uint24 length) -- we just have
 *             to detect unexpected Client Hello and Hello Request messages
 *             here, anything else is handled by higher layers
 *     Application data protocol
 *             none of our business
 */
int ssl3_read_bytes(SSL *s, int type, int *recvd_type, unsigned char *buf,
                    size_t len, int peek, size_t *readbytes)
{
    int i, j, ret;
    size_t n, curr_rec, num_recs, totalbytes;
    SSL3_RECORD *rr;
    SSL3_BUFFER *rbuf;
    void (*cb) (const SSL *ssl, int type2, int val) = NULL;
    int is_tls13 = SSL_IS_TLS13(s);

    rbuf = &s->rlayer.rbuf;

    if (!SSL3_BUFFER_is_initialised(rbuf)) {
        /* Not initialized yet */
        if (!ssl3_setup_read_buffer(s)) {
            /* SSLfatal() already called */
            return -1;
        }
    }

    if ((type && (type != SSL3_RT_APPLICATION_DATA)
         && (type != SSL3_RT_HANDSHAKE)) || (peek
                                             && (type !=
                                                 SSL3_RT_APPLICATION_DATA))) {
        SSLfatal(s, SSL_AD_INTERNAL_ERROR, ERR_R_INTERNAL_ERROR);
        return -1;
    }

    if ((type == SSL3_RT_HANDSHAKE) && (s->rlayer.handshake_fragment_len > 0))
        /* (partially) satisfy request from storage */
    {
        unsigned char *src = s->rlayer.handshake_fragment;
        unsigned char *dst = buf;
        unsigned int k;

        /* peek == 0 */
        n = 0;
        while ((len > 0) && (s->rlayer.handshake_fragment_len > 0)) {
            *dst++ = *src++;
            len--;
            s->rlayer.handshake_fragment_len--;
            n++;
        }
        /* move any remaining fragment bytes: */
        for (k = 0; k < s->rlayer.handshake_fragment_len; k++)
            s->rlayer.handshake_fragment[k] = *src++;

        if (recvd_type != NULL)
            *recvd_type = SSL3_RT_HANDSHAKE;

        *readbytes = n;
        return 1;
    }

    /*
     * Now s->rlayer.handshake_fragment_len == 0 if type == SSL3_RT_HANDSHAKE.
     */

    if (!ossl_statem_get_in_handshake(s) && SSL_in_init(s)) {
        /* type == SSL3_RT_APPLICATION_DATA */
        i = s->handshake_func(s);
        /* SSLfatal() already called */
        if (i < 0)
            return i;
        if (i == 0)
            return -1;
    }
 start:
    s->rwstate = SSL_NOTHING;

    /*-
     * For each record 'i' up to |num_recs]
     * rr[i].type     - is the type of record
     * rr[i].data,    - data
     * rr[i].off,     - offset into 'data' for next read
     * rr[i].length,  - number of bytes.
     */
    rr = s->rlayer.rrec;
    num_recs = RECORD_LAYER_get_numrpipes(&s->rlayer);

    do {
        /* get new records if necessary */
        if (num_recs == 0) {
            ret = ssl3_get_record(s);
            if (ret <= 0) {
                /* SSLfatal() already called if appropriate */
                return ret;
            }
            num_recs = RECORD_LAYER_get_numrpipes(&s->rlayer);
            if (num_recs == 0) {
                /* Shouldn't happen */
                SSLfatal(s, SSL_AD_INTERNAL_ERROR, ERR_R_INTERNAL_ERROR);
                return -1;
            }
        }
        /* Skip over any records we have already read */
        for (curr_rec = 0;
             curr_rec < num_recs && SSL3_RECORD_is_read(&rr[curr_rec]);
             curr_rec++) ;
        if (curr_rec == num_recs) {
            RECORD_LAYER_set_numrpipes(&s->rlayer, 0);
            num_recs = 0;
            curr_rec = 0;
        }
    } while (num_recs == 0);
    rr = &rr[curr_rec];

    if (s->rlayer.handshake_fragment_len > 0
            && SSL3_RECORD_get_type(rr) != SSL3_RT_HANDSHAKE
            && SSL_IS_TLS13(s)) {
        SSLfatal(s, SSL_AD_UNEXPECTED_MESSAGE,
                 SSL_R_MIXED_HANDSHAKE_AND_NON_HANDSHAKE_DATA);
        return -1;
    }

    /*
     * Reset the count of consecutive warning alerts if we've got a non-empty
     * record that isn't an alert.
     */
    if (SSL3_RECORD_get_type(rr) != SSL3_RT_ALERT
            && SSL3_RECORD_get_length(rr) != 0)
        s->rlayer.alert_count = 0;

    /* we now have a packet which can be read and processed */

    if (s->s3.change_cipher_spec /* set when we receive ChangeCipherSpec,
                                  * reset by ssl3_get_finished */
        && (SSL3_RECORD_get_type(rr) != SSL3_RT_HANDSHAKE)) {
        SSLfatal(s, SSL_AD_UNEXPECTED_MESSAGE,
                 SSL_R_DATA_BETWEEN_CCS_AND_FINISHED);
        return -1;
    }

    /*
     * If the other end has shut down, throw anything we read away (even in
     * 'peek' mode)
     */
    if (s->shutdown & SSL_RECEIVED_SHUTDOWN) {
        SSL3_RECORD_set_length(rr, 0);
        s->rwstate = SSL_NOTHING;
        return 0;
    }

    if (type == SSL3_RECORD_get_type(rr)
        || (SSL3_RECORD_get_type(rr) == SSL3_RT_CHANGE_CIPHER_SPEC
            && type == SSL3_RT_HANDSHAKE && recvd_type != NULL
            && !is_tls13)) {
        /*
         * SSL3_RT_APPLICATION_DATA or
         * SSL3_RT_HANDSHAKE or
         * SSL3_RT_CHANGE_CIPHER_SPEC
         */
        /*
         * make sure that we are not getting application data when we are
         * doing a handshake for the first time
         */
        if (SSL_in_init(s) && (type == SSL3_RT_APPLICATION_DATA) &&
            (s->enc_read_ctx == NULL)) {
            SSLfatal(s, SSL_AD_UNEXPECTED_MESSAGE, SSL_R_APP_DATA_IN_HANDSHAKE);
            return -1;
        }

        if (type == SSL3_RT_HANDSHAKE
            && SSL3_RECORD_get_type(rr) == SSL3_RT_CHANGE_CIPHER_SPEC
            && s->rlayer.handshake_fragment_len > 0) {
            SSLfatal(s, SSL_AD_UNEXPECTED_MESSAGE, SSL_R_CCS_RECEIVED_EARLY);
            return -1;
        }

        if (recvd_type != NULL)
            *recvd_type = SSL3_RECORD_get_type(rr);

        if (len == 0) {
            /*
             * Mark a zero length record as read. This ensures multiple calls to
             * SSL_read() with a zero length buffer will eventually cause
             * SSL_pending() to report data as being available.
             */
            if (SSL3_RECORD_get_length(rr) == 0)
                SSL3_RECORD_set_read(rr);
            return 0;
        }

        totalbytes = 0;
        do {
            if (len - totalbytes > SSL3_RECORD_get_length(rr))
                n = SSL3_RECORD_get_length(rr);
            else
                n = len - totalbytes;

            memcpy(buf, &(rr->data[rr->off]), n);
            buf += n;
            if (peek) {
                /* Mark any zero length record as consumed CVE-2016-6305 */
                if (SSL3_RECORD_get_length(rr) == 0)
                    SSL3_RECORD_set_read(rr);
            } else {
                if (s->options & SSL_OP_CLEANSE_PLAINTEXT)
                    OPENSSL_cleanse(&(rr->data[rr->off]), n);
                SSL3_RECORD_sub_length(rr, n);
                SSL3_RECORD_add_off(rr, n);
                if (SSL3_RECORD_get_length(rr) == 0) {
                    s->rlayer.rstate = SSL_ST_READ_HEADER;
                    SSL3_RECORD_set_off(rr, 0);
                    SSL3_RECORD_set_read(rr);
                }
            }
            if (SSL3_RECORD_get_length(rr) == 0
                || (peek && n == SSL3_RECORD_get_length(rr))) {
                curr_rec++;
                rr++;
            }
            totalbytes += n;
        } while (type == SSL3_RT_APPLICATION_DATA && curr_rec < num_recs
                 && totalbytes < len);
        if (totalbytes == 0) {
            /* We must have read empty records. Get more data */
            goto start;
        }
        if (!peek && curr_rec == num_recs
            && (s->mode & SSL_MODE_RELEASE_BUFFERS)
            && SSL3_BUFFER_get_left(rbuf) == 0)
            ssl3_release_read_buffer(s);
        *readbytes = totalbytes;
        return 1;
    }

    /*
     * If we get here, then type != rr->type; if we have a handshake message,
     * then it was unexpected (Hello Request or Client Hello) or invalid (we
     * were actually expecting a CCS).
     */

    /*
     * Lets just double check that we've not got an SSLv2 record
     */
    if (rr->rec_version == SSL2_VERSION) {
        /*
         * Should never happen. ssl3_get_record() should only give us an SSLv2
         * record back if this is the first packet and we are looking for an
         * initial ClientHello. Therefore |type| should always be equal to
         * |rr->type|. If not then something has gone horribly wrong
         */
        SSLfatal(s, SSL_AD_INTERNAL_ERROR, ERR_R_INTERNAL_ERROR);
        return -1;
    }

    if (s->method->version == TLS_ANY_VERSION
        && (s->server || rr->type != SSL3_RT_ALERT)) {
        /*
         * If we've got this far and still haven't decided on what version
         * we're using then this must be a client side alert we're dealing
         * with. We shouldn't be receiving anything other than a ClientHello
         * if we are a server.
         */
        s->version = rr->rec_version;
        SSLfatal(s, SSL_AD_UNEXPECTED_MESSAGE, SSL_R_UNEXPECTED_MESSAGE);
        return -1;
    }

    /*-
     * s->rlayer.handshake_fragment_len == 4  iff  rr->type == SSL3_RT_HANDSHAKE;
     * (Possibly rr is 'empty' now, i.e. rr->length may be 0.)
     */

    if (SSL3_RECORD_get_type(rr) == SSL3_RT_ALERT) {
        unsigned int alert_level, alert_descr;
        unsigned char *alert_bytes = SSL3_RECORD_get_data(rr)
                                     + SSL3_RECORD_get_off(rr);
        PACKET alert;

        if (!PACKET_buf_init(&alert, alert_bytes, SSL3_RECORD_get_length(rr))
                || !PACKET_get_1(&alert, &alert_level)
                || !PACKET_get_1(&alert, &alert_descr)
                || PACKET_remaining(&alert) != 0) {
            SSLfatal(s, SSL_AD_UNEXPECTED_MESSAGE, SSL_R_INVALID_ALERT);
            return -1;
        }

        if (s->msg_callback)
            s->msg_callback(0, s->version, SSL3_RT_ALERT, alert_bytes, 2, s,
                            s->msg_callback_arg);

        if (s->info_callback != NULL)
            cb = s->info_callback;
        else if (s->ctx->info_callback != NULL)
            cb = s->ctx->info_callback;

        if (cb != NULL) {
            j = (alert_level << 8) | alert_descr;
            cb(s, SSL_CB_READ_ALERT, j);
        }

        if (alert_level == SSL3_AL_WARNING
                || (is_tls13 && alert_descr == SSL_AD_USER_CANCELLED)) {
            s->s3.warn_alert = alert_descr;
            SSL3_RECORD_set_read(rr);

            s->rlayer.alert_count++;
            if (s->rlayer.alert_count == MAX_WARN_ALERT_COUNT) {
                SSLfatal(s, SSL_AD_UNEXPECTED_MESSAGE,
                         SSL_R_TOO_MANY_WARN_ALERTS);
                return -1;
            }
        }

        /*
         * Apart from close_notify the only other warning alert in TLSv1.3
         * is user_cancelled - which we just ignore.
         */
        if (is_tls13 && alert_descr == SSL_AD_USER_CANCELLED) {
            goto start;
        } else if (alert_descr == SSL_AD_CLOSE_NOTIFY
                && (is_tls13 || alert_level == SSL3_AL_WARNING)) {
            s->shutdown |= SSL_RECEIVED_SHUTDOWN;
            return 0;
        } else if (alert_level == SSL3_AL_FATAL || is_tls13) {
            s->rwstate = SSL_NOTHING;
            s->s3.fatal_alert = alert_descr;
            SSLfatal_data(s, SSL_AD_NO_ALERT,
                          SSL_AD_REASON_OFFSET + alert_descr,
                          "SSL alert number %d", alert_descr);
            s->shutdown |= SSL_RECEIVED_SHUTDOWN;
            SSL3_RECORD_set_read(rr);
            SSL_CTX_remove_session(s->session_ctx, s->session);
            return 0;
        } else if (alert_descr == SSL_AD_NO_RENEGOTIATION) {
            /*
             * This is a warning but we receive it if we requested
             * renegotiation and the peer denied it. Terminate with a fatal
             * alert because if application tried to renegotiate it
             * presumably had a good reason and expects it to succeed. In
             * future we might have a renegotiation where we don't care if
             * the peer refused it where we carry on.
             */
            SSLfatal(s, SSL_AD_HANDSHAKE_FAILURE, SSL_R_NO_RENEGOTIATION);
            return -1;
        } else if (alert_level == SSL3_AL_WARNING) {
            /* We ignore any other warning alert in TLSv1.2 and below */
            goto start;
        }

        SSLfatal(s, SSL_AD_ILLEGAL_PARAMETER, SSL_R_UNKNOWN_ALERT_TYPE);
        return -1;
    }

    if ((s->shutdown & SSL_SENT_SHUTDOWN) != 0) {
        if (SSL3_RECORD_get_type(rr) == SSL3_RT_HANDSHAKE) {
            BIO *rbio;

            /*
             * We ignore any handshake messages sent to us unless they are
             * TLSv1.3 in which case we want to process them. For all other
             * handshake messages we can't do anything reasonable with them
             * because we are unable to write any response due to having already
             * sent close_notify.
             */
            if (!SSL_IS_TLS13(s)) {
                SSL3_RECORD_set_length(rr, 0);
                SSL3_RECORD_set_read(rr);

                if ((s->mode & SSL_MODE_AUTO_RETRY) != 0)
                    goto start;

                s->rwstate = SSL_READING;
                rbio = SSL_get_rbio(s);
                BIO_clear_retry_flags(rbio);
                BIO_set_retry_read(rbio);
                return -1;
            }
        } else {
            /*
             * The peer is continuing to send application data, but we have
             * already sent close_notify. If this was expected we should have
             * been called via SSL_read() and this would have been handled
             * above.
             * No alert sent because we already sent close_notify
             */
            SSL3_RECORD_set_length(rr, 0);
            SSL3_RECORD_set_read(rr);
            SSLfatal(s, SSL_AD_NO_ALERT,
                     SSL_R_APPLICATION_DATA_AFTER_CLOSE_NOTIFY);
            return -1;
        }
    }

    /*
     * For handshake data we have 'fragment' storage, so fill that so that we
     * can process the header at a fixed place. This is done after the
     * "SHUTDOWN" code above to avoid filling the fragment storage with data
     * that we're just going to discard.
     */
    if (SSL3_RECORD_get_type(rr) == SSL3_RT_HANDSHAKE) {
        size_t dest_maxlen = sizeof(s->rlayer.handshake_fragment);
        unsigned char *dest = s->rlayer.handshake_fragment;
        size_t *dest_len = &s->rlayer.handshake_fragment_len;

        n = dest_maxlen - *dest_len; /* available space in 'dest' */
        if (SSL3_RECORD_get_length(rr) < n)
            n = SSL3_RECORD_get_length(rr); /* available bytes */

        /* now move 'n' bytes: */
        memcpy(dest + *dest_len,
               SSL3_RECORD_get_data(rr) + SSL3_RECORD_get_off(rr), n);
        SSL3_RECORD_add_off(rr, n);
        SSL3_RECORD_sub_length(rr, n);
        *dest_len += n;
        if (SSL3_RECORD_get_length(rr) == 0)
            SSL3_RECORD_set_read(rr);

        if (*dest_len < dest_maxlen)
            goto start;     /* fragment was too small */
    }

    if (SSL3_RECORD_get_type(rr) == SSL3_RT_CHANGE_CIPHER_SPEC) {
        SSLfatal(s, SSL_AD_UNEXPECTED_MESSAGE, SSL_R_CCS_RECEIVED_EARLY);
        return -1;
    }

    /*
     * Unexpected handshake message (ClientHello, NewSessionTicket (TLS1.3) or
     * protocol violation)
     */
    if ((s->rlayer.handshake_fragment_len >= 4)
            && !ossl_statem_get_in_handshake(s)) {
        int ined = (s->early_data_state == SSL_EARLY_DATA_READING);

        /* We found handshake data, so we're going back into init */
        ossl_statem_set_in_init(s, 1);

        i = s->handshake_func(s);
        /* SSLfatal() already called if appropriate */
        if (i < 0)
            return i;
        if (i == 0) {
            return -1;
        }

        /*
         * If we were actually trying to read early data and we found a
         * handshake message, then we don't want to continue to try and read
         * the application data any more. It won't be "early" now.
         */
        if (ined)
            return -1;

        if (!(s->mode & SSL_MODE_AUTO_RETRY)) {
            if (SSL3_BUFFER_get_left(rbuf) == 0) {
                /* no read-ahead left? */
                BIO *bio;
                /*
                 * In the case where we try to read application data, but we
                 * trigger an SSL handshake, we return -1 with the retry
                 * option set.  Otherwise renegotiation may cause nasty
                 * problems in the blocking world
                 */
                s->rwstate = SSL_READING;
                bio = SSL_get_rbio(s);
                BIO_clear_retry_flags(bio);
                BIO_set_retry_read(bio);
                return -1;
            }
        }
        goto start;
    }

    switch (SSL3_RECORD_get_type(rr)) {
    default:
        /*
         * TLS 1.0 and 1.1 say you SHOULD ignore unrecognised record types, but
         * TLS 1.2 says you MUST send an unexpected message alert. We use the
         * TLS 1.2 behaviour for all protocol versions to prevent issues where
         * no progress is being made and the peer continually sends unrecognised
         * record types, using up resources processing them.
         */
        SSLfatal(s, SSL_AD_UNEXPECTED_MESSAGE, SSL_R_UNEXPECTED_RECORD);
        return -1;
    case SSL3_RT_CHANGE_CIPHER_SPEC:
    case SSL3_RT_ALERT:
    case SSL3_RT_HANDSHAKE:
        /*
         * we already handled all of these, with the possible exception of
         * SSL3_RT_HANDSHAKE when ossl_statem_get_in_handshake(s) is true, but
         * that should not happen when type != rr->type
         */
        SSLfatal(s, SSL_AD_UNEXPECTED_MESSAGE, ERR_R_INTERNAL_ERROR);
        return -1;
    case SSL3_RT_APPLICATION_DATA:
        /*
         * At this point, we were expecting handshake data, but have
         * application data.  If the library was running inside ssl3_read()
         * (i.e. in_read_app_data is set) and it makes sense to read
         * application data at this point (session renegotiation not yet
         * started), we will indulge it.
         */
        if (ossl_statem_app_data_allowed(s)) {
            s->s3.in_read_app_data = 2;
            return -1;
        } else if (ossl_statem_skip_early_data(s)) {
            /*
             * This can happen after a client sends a CH followed by early_data,
             * but the server responds with a HelloRetryRequest. The server
             * reads the next record from the client expecting to find a
             * plaintext ClientHello but gets a record which appears to be
             * application data. The trial decrypt "works" because null
             * decryption was applied. We just skip it and move on to the next
             * record.
             */
            if (!early_data_count_ok(s, rr->length,
                                     EARLY_DATA_CIPHERTEXT_OVERHEAD, 0)) {
                /* SSLfatal() already called */
                return -1;
            }
            SSL3_RECORD_set_read(rr);
            goto start;
        } else {
            SSLfatal(s, SSL_AD_UNEXPECTED_MESSAGE, SSL_R_UNEXPECTED_RECORD);
            return -1;
        }
    }
}

void ssl3_record_sequence_update(unsigned char *seq)
{
    int i;

    for (i = 7; i >= 0; i--) {
        ++seq[i];
        if (seq[i] != 0)
            break;
    }
}

/*
 * Returns true if the current rrec was sent in SSLv2 backwards compatible
 * format and false otherwise.
 */
int RECORD_LAYER_is_sslv2_record(RECORD_LAYER *rl)
{
    return SSL3_RECORD_is_sslv2_record(&rl->rrec[0]);
}

/*
 * Returns the length in bytes of the current rrec
 */
size_t RECORD_LAYER_get_rrec_length(RECORD_LAYER *rl)
{
    return SSL3_RECORD_get_length(&rl->rrec[0]);
}