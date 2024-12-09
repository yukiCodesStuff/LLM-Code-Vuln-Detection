#include "statem_local.h"
#include "internal/cryptlib.h"

#define COOKIE_STATE_FORMAT_VERSION     0

/*
 * 2 bytes for packet length, 2 bytes for format version, 2 bytes for
 * protocol version, 2 bytes for group id, 2 bytes for cipher id, 1 byte for
 * key_share present flag, 4 bytes for timestamp, 2 bytes for the hashlen,
 * EVP_MAX_MD_SIZE for transcript hash, 1 byte for app cookie length, app cookie
 * length bytes, SHA256_DIGEST_LENGTH bytes for the HMAC of the whole thing.
 */
#define MAX_COOKIE_SIZE (2 + 2 + 2 + 2 + 2 + 1 + 4 + 2 + EVP_MAX_MD_SIZE + 1 \
                         + SSL_COOKIE_LENGTH + SHA256_DIGEST_LENGTH)

/*
 * Message header + 2 bytes for protocol version + number of random bytes +
        }

        /* Check if this share is for a group we can use */
        if (!check_in_list(s, group_id, srvrgroups, srvr_num_groups, 1)) {
            /* Share not suitable */
            continue;
        }

    unsigned char hmac[SHA256_DIGEST_LENGTH];
    unsigned char hrr[MAX_HRR_SIZE];
    size_t rawlen, hmaclen, hrrlen, ciphlen;
    unsigned long tm, now;

    /* Ignore any cookie if we're not set up to verify it */
    if (s->ctx->verify_stateless_cookie_cb == NULL
            || (s->s3.flags & TLS1_FLAGS_STATELESS) == 0)
    }

    if (!PACKET_get_1(&cookie, &key_share)
            || !PACKET_get_net_4(&cookie, &tm)
            || !PACKET_get_length_prefixed_2(&cookie, &chhash)
            || !PACKET_get_length_prefixed_1(&cookie, &appcookie)
            || PACKET_remaining(&cookie) != SHA256_DIGEST_LENGTH) {
        SSLfatal(s, SSL_AD_DECODE_ERROR, SSL_R_LENGTH_MISMATCH);
    }

    /* We tolerate a cookie age of up to 10 minutes (= 60 * 10 seconds) */
    now = (unsigned long)time(NULL);
    if (tm > now || (now - tm) > 600) {
        /* Cookie is stale. Ignore it */
        return 1;
    }
                s->ext.early_data_ok = 1;
            s->ext.ticket_expected = 1;
        } else {
            uint32_t ticket_age = 0, now, agesec, agems;
            int ret;

            /*
             * If we are using anti-replay protection then we behave as if
            }

            ticket_age = (uint32_t)ticket_agel;
            now = (uint32_t)time(NULL);
            agesec = now - (uint32_t)sess->time;
            agems = agesec * (uint32_t)1000;
            ticket_age -= sess->ext.tick_age_add;

            /*
        }

        md = ssl_md(s->ctx, sess->cipher->algorithm2);
        if (!EVP_MD_is_a(md,
                EVP_MD_get0_name(ssl_md(s->ctx,
                                        s->s3.tmp.new_cipher->algorithm2)))) {
            /* The ciphersuite is not compatible with this session. */
                                              &ciphlen)
               /* Is there a key_share extension present in this HRR? */
            || !WPACKET_put_bytes_u8(pkt, s->s3.peer_tmp == NULL)
            || !WPACKET_put_bytes_u32(pkt, (unsigned int)time(NULL))
            || !WPACKET_start_sub_packet_u16(pkt)
            || !WPACKET_reserve_bytes(pkt, EVP_MAX_MD_SIZE, &hashval1)) {
        SSLfatal(s, SSL_AD_INTERNAL_ERROR, ERR_R_INTERNAL_ERROR);
        return EXT_RETURN_FAIL;