/*
 * Copyright 2016-2022 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#include <string.h>

#include "internal/nelem.h"
#include "ssltestlib.h"
#include "../testutil.h"
#include "e_os.h" /* for ossl_sleep() etc. */

#ifdef OPENSSL_SYS_UNIX
# include <unistd.h>
# ifndef OPENSSL_NO_KTLS
#  include <netinet/in.h>
#  include <netinet/in.h>
#  include <arpa/inet.h>
#  include <sys/socket.h>
#  include <unistd.h>
#  include <fcntl.h>
# endif
#endif

static int tls_dump_new(BIO *bi);
static int tls_dump_free(BIO *a);
static int tls_dump_read(BIO *b, char *out, int outl);
static int tls_dump_write(BIO *b, const char *in, int inl);
static long tls_dump_ctrl(BIO *b, int cmd, long num, void *ptr);
static int tls_dump_gets(BIO *bp, char *buf, int size);
static int tls_dump_puts(BIO *bp, const char *str);

/* Choose a sufficiently large type likely to be unused for this custom BIO */
#define BIO_TYPE_TLS_DUMP_FILTER  (0x80 | BIO_TYPE_FILTER)
#define BIO_TYPE_MEMPACKET_TEST    0x81
#define BIO_TYPE_ALWAYS_RETRY      0x82

static BIO_METHOD *method_tls_dump = NULL;
static BIO_METHOD *meth_mem = NULL;
static BIO_METHOD *meth_always_retry = NULL;

/* Note: Not thread safe! */
const BIO_METHOD *bio_f_tls_dump_filter(void)
{
    if (method_tls_dump == NULL) {
        method_tls_dump = BIO_meth_new(BIO_TYPE_TLS_DUMP_FILTER,
                                        "TLS dump filter");
        if (   method_tls_dump == NULL
            || !BIO_meth_set_write(method_tls_dump, tls_dump_write)
            || !BIO_meth_set_read(method_tls_dump, tls_dump_read)
            || !BIO_meth_set_puts(method_tls_dump, tls_dump_puts)
            || !BIO_meth_set_gets(method_tls_dump, tls_dump_gets)
            || !BIO_meth_set_ctrl(method_tls_dump, tls_dump_ctrl)
            || !BIO_meth_set_create(method_tls_dump, tls_dump_new)
            || !BIO_meth_set_destroy(method_tls_dump, tls_dump_free))
            return NULL;
    }
    return method_tls_dump;
}
{
    MEMPACKET_TEST_CTX *ctx = BIO_get_data(bio);
    MEMPACKET *thispkt;
    unsigned char *rec;
    int rem;
    unsigned int seq, offset, len, epoch;

    BIO_clear_retry_flags(bio);
    if ((thispkt = sk_MEMPACKET_value(ctx->pkts, 0)) == NULL
        || thispkt->num != ctx->currpkt) {
        /* Probably run out of data */
        BIO_set_retry_read(bio);
        return -1;
    }
            } else {
                rec += len;
            }

            ctx->currrec++;
        }
    }

    memcpy(out, thispkt->data, outl);
    mempacket_free(thispkt);
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
    MEMPACKET *thispkt = NULL, *looppkt, *nextpkt, *allpkts[3];
    int i, duprec;
    const unsigned char *inu = (const unsigned char *)in;
    size_t len = ((inu[RECORD_LEN_HI] << 8) | inu[RECORD_LEN_LO])
                 + DTLS1_RT_HEADER_LENGTH;

    if (ctx == NULL)
        return -1;

    if ((size_t)inl < len)
        return -1;

    if ((size_t)inl == len)
        duprec = 0;
    else
        duprec = ctx->duprec > 0;

    /* We don't support arbitrary injection when duplicating records */
    if (duprec && pktnum != -1)
        return -1;

    /* We only allow injection before we've started writing any data */
    if (pktnum >= 0) {
        if (ctx->noinject)
            return -1;
        ctx->injected  = 1;
    } else {
        ctx->noinject = 1;
    }

    for (i = 0; i < (duprec ? 3 : 1); i++) {
        if (!TEST_ptr(allpkts[i] = OPENSSL_malloc(sizeof(*thispkt))))
            goto err;
        thispkt = allpkts[i];

        if (!TEST_ptr(thispkt->data = OPENSSL_malloc(inl)))
            goto err;
        /*
         * If we are duplicating the packet, we duplicate it three times. The
         * first two times we drop the first record if there are more than one.
         * In this way we know that libssl will not be able to make progress
         * until it receives the last packet, and hence will be forced to
         * buffer these records.
         */
        if (duprec && i != 2) {
            memcpy(thispkt->data, in + len, inl - len);
            thispkt->len = inl - len;
        } else {
            memcpy(thispkt->data, in, inl);
            thispkt->len = inl;
        }
        thispkt->num = (pktnum >= 0) ? (unsigned int)pktnum : ctx->lastpkt + i;
        thispkt->type = type;
    }

    for (i = 0; i < sk_MEMPACKET_num(ctx->pkts); i++) {
        if (!TEST_ptr(looppkt = sk_MEMPACKET_value(ctx->pkts, i)))
            goto err;
        /* Check if we found the right place to insert this packet */
        if (looppkt->num > thispkt->num) {
            if (sk_MEMPACKET_insert(ctx->pkts, thispkt, i) == 0)
                goto err;
            /* If we're doing up front injection then we're done */
            if (pktnum >= 0)
                return inl;
            /*
             * We need to do some accounting on lastpkt. We increment it first,
             * but it might now equal the value of injected packets, so we need
             * to skip over those
             */
            ctx->lastpkt++;
            do {
                i++;
                nextpkt = sk_MEMPACKET_value(ctx->pkts, i);
                if (nextpkt != NULL && nextpkt->num == ctx->lastpkt)
                    ctx->lastpkt++;
                else
                    return inl;
            } while(1);
        } else if (looppkt->num == thispkt->num) {
            if (!ctx->noinject) {
                /* We injected two packets with the same packet number! */
                goto err;
            }
            ctx->lastpkt++;
            thispkt->num++;
        }
    }
    /*
     * We didn't find any packets with a packet number equal to or greater than
     * this one, so we just add it onto the end
     */
    for (i = 0; i < (duprec ? 3 : 1); i++) {
        thispkt = allpkts[i];
        if (!sk_MEMPACKET_push(ctx->pkts, thispkt))
            goto err;

        if (pktnum < 0)
            ctx->lastpkt++;
    }

    return inl;

 err:
    for (i = 0; i < (ctx->duprec > 0 ? 3 : 1); i++)
        mempacket_free(allpkts[i]);
    return -1;
}

static int mempacket_test_write(BIO *bio, const char *in, int inl)
{
    return mempacket_test_inject(bio, in, inl, -1, STANDARD_PACKET);
}

static long mempacket_test_ctrl(BIO *bio, int cmd, long num, void *ptr)
{
    long ret = 1;
    MEMPACKET_TEST_CTX *ctx = BIO_get_data(bio);
    MEMPACKET *thispkt;

    switch (cmd) {
    case BIO_CTRL_EOF:
        ret = (long)(sk_MEMPACKET_num(ctx->pkts) == 0);
        break;
    case BIO_CTRL_GET_CLOSE:
        ret = BIO_get_shutdown(bio);
        break;
    case BIO_CTRL_SET_CLOSE:
        BIO_set_shutdown(bio, (int)num);
        break;
    case BIO_CTRL_WPENDING:
        ret = 0L;
        break;
    case BIO_CTRL_PENDING:
        thispkt = sk_MEMPACKET_value(ctx->pkts, 0);
        if (thispkt == NULL)
            ret = 0;
        else
            ret = thispkt->len;
        break;
    case BIO_CTRL_FLUSH:
        ret = 1;
        break;
    case MEMPACKET_CTRL_SET_DROP_EPOCH:
        ctx->dropepoch = (unsigned int)num;
        break;
    case MEMPACKET_CTRL_SET_DROP_REC:
        ctx->droprec = (int)num;
        break;
    case MEMPACKET_CTRL_GET_DROP_REC:
        ret = ctx->droprec;
        break;
    case MEMPACKET_CTRL_SET_DUPLICATE_REC:
        ctx->duprec = (int)num;
        break;
    case BIO_CTRL_RESET:
    case BIO_CTRL_DUP:
    case BIO_CTRL_PUSH:
    case BIO_CTRL_POP:
    default:
        ret = 0;
        break;
    }
    return ret;
}

static int mempacket_test_gets(BIO *bio, char *buf, int size)
{
    /* We don't support this - not needed anyway */
    return -1;
}

static int mempacket_test_puts(BIO *bio, const char *str)
{
    return mempacket_test_write(bio, str, strlen(str));
}

static int always_retry_new(BIO *bi);
static int always_retry_free(BIO *a);
static int always_retry_read(BIO *b, char *out, int outl);
static int always_retry_write(BIO *b, const char *in, int inl);
static long always_retry_ctrl(BIO *b, int cmd, long num, void *ptr);
static int always_retry_gets(BIO *bp, char *buf, int size);
static int always_retry_puts(BIO *bp, const char *str);

const BIO_METHOD *bio_s_always_retry(void)
{
    if (meth_always_retry == NULL) {
        if (!TEST_ptr(meth_always_retry = BIO_meth_new(BIO_TYPE_ALWAYS_RETRY,
                                                       "Always Retry"))
            || !TEST_true(BIO_meth_set_write(meth_always_retry,
                                             always_retry_write))
            || !TEST_true(BIO_meth_set_read(meth_always_retry,
                                            always_retry_read))
            || !TEST_true(BIO_meth_set_puts(meth_always_retry,
                                            always_retry_puts))
            || !TEST_true(BIO_meth_set_gets(meth_always_retry,
                                            always_retry_gets))
            || !TEST_true(BIO_meth_set_ctrl(meth_always_retry,
                                            always_retry_ctrl))
            || !TEST_true(BIO_meth_set_create(meth_always_retry,
                                              always_retry_new))
            || !TEST_true(BIO_meth_set_destroy(meth_always_retry,
                                               always_retry_free)))
            return NULL;
    }
    return meth_always_retry;
}

void bio_s_always_retry_free(void)
{
    BIO_meth_free(meth_always_retry);
}

static int always_retry_new(BIO *bio)
{
    BIO_set_init(bio, 1);
    return 1;
}

static int always_retry_free(BIO *bio)
{
    BIO_set_data(bio, NULL);
    BIO_set_init(bio, 0);
    return 1;
}

static int always_retry_read(BIO *bio, char *out, int outl)
{
    BIO_set_retry_read(bio);
    return -1;
}

static int always_retry_write(BIO *bio, const char *in, int inl)
{
    BIO_set_retry_write(bio);
    return -1;
}

static long always_retry_ctrl(BIO *bio, int cmd, long num, void *ptr)
{
    long ret = 1;

    switch (cmd) {
    case BIO_CTRL_FLUSH:
        BIO_set_retry_write(bio);
        /* fall through */
    case BIO_CTRL_EOF:
    case BIO_CTRL_RESET:
    case BIO_CTRL_DUP:
    case BIO_CTRL_PUSH:
    case BIO_CTRL_POP:
    default:
        ret = 0;
        break;
    }
    return ret;
}

static int always_retry_gets(BIO *bio, char *buf, int size)
{
    BIO_set_retry_read(bio);
    return -1;
}

static int always_retry_puts(BIO *bio, const char *str)
{
    BIO_set_retry_write(bio);
    return -1;
}

int create_ssl_ctx_pair(OSSL_LIB_CTX *libctx, const SSL_METHOD *sm,
                        const SSL_METHOD *cm, int min_proto_version,
                        int max_proto_version, SSL_CTX **sctx, SSL_CTX **cctx,
                        char *certfile, char *privkeyfile)
{
    SSL_CTX *serverctx = NULL;
    SSL_CTX *clientctx = NULL;

    if (sctx != NULL) {
        if (*sctx != NULL)
            serverctx = *sctx;
        else if (!TEST_ptr(serverctx = SSL_CTX_new_ex(libctx, NULL, sm))
            || !TEST_true(SSL_CTX_set_options(serverctx,
                                              SSL_OP_ALLOW_CLIENT_RENEGOTIATION)))
            goto err;
    }

    if (cctx != NULL) {
        if (*cctx != NULL)
            clientctx = *cctx;
        else if (!TEST_ptr(clientctx = SSL_CTX_new_ex(libctx, NULL, cm)))
            goto err;
    }

#if !defined(OPENSSL_NO_TLS1_3) \
    && defined(OPENSSL_NO_EC) \
    && defined(OPENSSL_NO_DH)
    /*
     * There are no usable built-in TLSv1.3 groups if ec and dh are both
     * disabled
     */
    if (max_proto_version == 0
            && (sm == TLS_server_method() || cm == TLS_client_method()))
        max_proto_version = TLS1_2_VERSION;
#endif

    if (serverctx != NULL
            && ((min_proto_version > 0
                 && !TEST_true(SSL_CTX_set_min_proto_version(serverctx,
                                                            min_proto_version)))
                || (max_proto_version > 0
                    && !TEST_true(SSL_CTX_set_max_proto_version(serverctx,
                                                                max_proto_version)))))
        goto err;
    if (clientctx != NULL
        && ((min_proto_version > 0
             && !TEST_true(SSL_CTX_set_min_proto_version(clientctx,
                                                         min_proto_version)))
            || (max_proto_version > 0
                && !TEST_true(SSL_CTX_set_max_proto_version(clientctx,
                                                            max_proto_version)))))
        goto err;

    if (serverctx != NULL && certfile != NULL && privkeyfile != NULL) {
        if (!TEST_int_eq(SSL_CTX_use_certificate_file(serverctx, certfile,
                                                      SSL_FILETYPE_PEM), 1)
                || !TEST_int_eq(SSL_CTX_use_PrivateKey_file(serverctx,
                                                            privkeyfile,
                                                            SSL_FILETYPE_PEM), 1)
                || !TEST_int_eq(SSL_CTX_check_private_key(serverctx), 1))
            goto err;
    }

    if (sctx != NULL)
        *sctx = serverctx;
    if (cctx != NULL)
        *cctx = clientctx;
    return 1;

 err:
    if (sctx != NULL && *sctx == NULL)
        SSL_CTX_free(serverctx);
    if (cctx != NULL && *cctx == NULL)
        SSL_CTX_free(clientctx);
    return 0;
}

#define MAXLOOPS    1000000

#if !defined(OPENSSL_NO_KTLS) && !defined(OPENSSL_NO_SOCK)
static int set_nb(int fd)
{
    int flags;

    flags = fcntl(fd,F_GETFL,0);
    if (flags == -1)
        return flags;
    flags = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    return flags;
}

int create_test_sockets(int *cfdp, int *sfdp)
{
    struct sockaddr_in sin;
    const char *host = "127.0.0.1";
    int cfd_connected = 0, ret = 0;
    socklen_t slen = sizeof(sin);
    int afd = -1, cfd = -1, sfd = -1;

    memset ((char *) &sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = inet_addr(host);

    afd = socket(AF_INET, SOCK_STREAM, 0);
    if (afd < 0)
        return 0;

    if (bind(afd, (struct sockaddr*)&sin, sizeof(sin)) < 0)
        goto out;

    if (getsockname(afd, (struct sockaddr*)&sin, &slen) < 0)
        goto out;

    if (listen(afd, 1) < 0)
        goto out;

    cfd = socket(AF_INET, SOCK_STREAM, 0);
    if (cfd < 0)
        goto out;

    if (set_nb(afd) == -1)
        goto out;

    while (sfd == -1 || !cfd_connected ) {
        sfd = accept(afd, NULL, 0);
        if (sfd == -1 && errno != EAGAIN)
            goto out;

        if (!cfd_connected && connect(cfd, (struct sockaddr*)&sin, sizeof(sin)) < 0)
            goto out;
        else
            cfd_connected = 1;
    }

    if (set_nb(cfd) == -1 || set_nb(sfd) == -1)
        goto out;
    ret = 1;
    *cfdp = cfd;
    *sfdp = sfd;
    goto success;

out:
    if (cfd != -1)
        close(cfd);
    if (sfd != -1)
        close(sfd);
success:
    if (afd != -1)
        close(afd);
    return ret;
}

int create_ssl_objects2(SSL_CTX *serverctx, SSL_CTX *clientctx, SSL **sssl,
                          SSL **cssl, int sfd, int cfd)
{
    SSL *serverssl = NULL, *clientssl = NULL;
    BIO *s_to_c_bio = NULL, *c_to_s_bio = NULL;

    if (*sssl != NULL)
        serverssl = *sssl;
    else if (!TEST_ptr(serverssl = SSL_new(serverctx)))
        goto error;
    if (*cssl != NULL)
        clientssl = *cssl;
    else if (!TEST_ptr(clientssl = SSL_new(clientctx)))
        goto error;

    if (!TEST_ptr(s_to_c_bio = BIO_new_socket(sfd, BIO_NOCLOSE))
            || !TEST_ptr(c_to_s_bio = BIO_new_socket(cfd, BIO_NOCLOSE)))
        goto error;

    SSL_set_bio(clientssl, c_to_s_bio, c_to_s_bio);
    SSL_set_bio(serverssl, s_to_c_bio, s_to_c_bio);
    *sssl = serverssl;
    *cssl = clientssl;
    return 1;

 error:
    SSL_free(serverssl);
    SSL_free(clientssl);
    BIO_free(s_to_c_bio);
    BIO_free(c_to_s_bio);
    return 0;
}
#endif

/*
 * NOTE: Transfers control of the BIOs - this function will free them on error
 */
int create_ssl_objects(SSL_CTX *serverctx, SSL_CTX *clientctx, SSL **sssl,
                          SSL **cssl, BIO *s_to_c_fbio, BIO *c_to_s_fbio)
{
    SSL *serverssl = NULL, *clientssl = NULL;
    BIO *s_to_c_bio = NULL, *c_to_s_bio = NULL;

    if (*sssl != NULL)
        serverssl = *sssl;
    else if (!TEST_ptr(serverssl = SSL_new(serverctx)))
        goto error;
    if (*cssl != NULL)
        clientssl = *cssl;
    else if (!TEST_ptr(clientssl = SSL_new(clientctx)))
        goto error;

    if (SSL_is_dtls(clientssl)) {
        if (!TEST_ptr(s_to_c_bio = BIO_new(bio_s_mempacket_test()))
                || !TEST_ptr(c_to_s_bio = BIO_new(bio_s_mempacket_test())))
            goto error;
    } else {
        if (!TEST_ptr(s_to_c_bio = BIO_new(BIO_s_mem()))
                || !TEST_ptr(c_to_s_bio = BIO_new(BIO_s_mem())))
            goto error;
    }

    if (s_to_c_fbio != NULL
            && !TEST_ptr(s_to_c_bio = BIO_push(s_to_c_fbio, s_to_c_bio)))
        goto error;
    if (c_to_s_fbio != NULL
            && !TEST_ptr(c_to_s_bio = BIO_push(c_to_s_fbio, c_to_s_bio)))
        goto error;

    /* Set Non-blocking IO behaviour */
    BIO_set_mem_eof_return(s_to_c_bio, -1);
    BIO_set_mem_eof_return(c_to_s_bio, -1);

    /* Up ref these as we are passing them to two SSL objects */
    SSL_set_bio(serverssl, c_to_s_bio, s_to_c_bio);
    BIO_up_ref(s_to_c_bio);
    BIO_up_ref(c_to_s_bio);
    SSL_set_bio(clientssl, s_to_c_bio, c_to_s_bio);
    *sssl = serverssl;
    *cssl = clientssl;
    return 1;

 error:
    SSL_free(serverssl);
    SSL_free(clientssl);
    BIO_free(s_to_c_bio);
    BIO_free(c_to_s_bio);
    BIO_free(s_to_c_fbio);
    BIO_free(c_to_s_fbio);

    return 0;
}

/*
 * Create an SSL connection, but does not read any post-handshake
 * NewSessionTicket messages.
 * If |read| is set and we're using DTLS then we will attempt to SSL_read on
 * the connection once we've completed one half of it, to ensure any retransmits
 * get triggered.
 * We stop the connection attempt (and return a failure value) if either peer
 * has SSL_get_error() return the value in the |want| parameter. The connection
 * attempt could be restarted by a subsequent call to this function.
 */
int create_bare_ssl_connection(SSL *serverssl, SSL *clientssl, int want,
                               int read)
{
    int retc = -1, rets = -1, err, abortctr = 0;
    int clienterr = 0, servererr = 0;
    int isdtls = SSL_is_dtls(serverssl);

    do {
        err = SSL_ERROR_WANT_WRITE;
        while (!clienterr && retc <= 0 && err == SSL_ERROR_WANT_WRITE) {
            retc = SSL_connect(clientssl);
            if (retc <= 0)
                err = SSL_get_error(clientssl, retc);
        }

        if (!clienterr && retc <= 0 && err != SSL_ERROR_WANT_READ) {
            TEST_info("SSL_connect() failed %d, %d", retc, err);
            if (want != SSL_ERROR_SSL)
                TEST_openssl_errors();
            clienterr = 1;
        }
        if (want != SSL_ERROR_NONE && err == want)
            return 0;

        err = SSL_ERROR_WANT_WRITE;
        while (!servererr && rets <= 0 && err == SSL_ERROR_WANT_WRITE) {
            rets = SSL_accept(serverssl);
            if (rets <= 0)
                err = SSL_get_error(serverssl, rets);
        }

        if (!servererr && rets <= 0
                && err != SSL_ERROR_WANT_READ
                && err != SSL_ERROR_WANT_X509_LOOKUP) {
            TEST_info("SSL_accept() failed %d, %d", rets, err);
            if (want != SSL_ERROR_SSL)
                TEST_openssl_errors();
            servererr = 1;
        }
        if (want != SSL_ERROR_NONE && err == want)
            return 0;
        if (clienterr && servererr)
            return 0;
        if (isdtls && read) {
            unsigned char buf[20];

            /* Trigger any retransmits that may be appropriate */
            if (rets > 0 && retc <= 0) {
                if (SSL_read(serverssl, buf, sizeof(buf)) > 0) {
                    /* We don't expect this to succeed! */
                    TEST_info("Unexpected SSL_read() success!");
                    return 0;
                }
            }
            if (retc > 0 && rets <= 0) {
                if (SSL_read(clientssl, buf, sizeof(buf)) > 0) {
                    /* We don't expect this to succeed! */
                    TEST_info("Unexpected SSL_read() success!");
                    return 0;
                }
            }
        }
        if (++abortctr == MAXLOOPS) {
            TEST_info("No progress made");
            return 0;
        }
        if (isdtls && abortctr <= 50 && (abortctr % 10) == 0) {
            /*
             * It looks like we're just spinning. Pause for a short period to
             * give the DTLS timer a chance to do something. We only do this for
             * the first few times to prevent hangs.
             */
            ossl_sleep(50);
        }
    } while (retc <=0 || rets <= 0);

    return 1;
}

/*
 * Create an SSL connection including any post handshake NewSessionTicket
 * messages.
 */
int create_ssl_connection(SSL *serverssl, SSL *clientssl, int want)
{
    int i;
    unsigned char buf;
    size_t readbytes;

    if (!create_bare_ssl_connection(serverssl, clientssl, want, 1))
        return 0;

    /*
     * We attempt to read some data on the client side which we expect to fail.
     * This will ensure we have received the NewSessionTicket in TLSv1.3 where
     * appropriate. We do this twice because there are 2 NewSessionTickets.
     */
    for (i = 0; i < 2; i++) {
        if (SSL_read_ex(clientssl, &buf, sizeof(buf), &readbytes) > 0) {
            if (!TEST_ulong_eq(readbytes, 0))
                return 0;
        } else if (!TEST_int_eq(SSL_get_error(clientssl, 0),
                                SSL_ERROR_WANT_READ)) {
            return 0;
        }
    }

    return 1;
}

void shutdown_ssl_connection(SSL *serverssl, SSL *clientssl)
{
    SSL_shutdown(clientssl);
    SSL_shutdown(serverssl);
    SSL_free(serverssl);
    SSL_free(clientssl);
}
        } else {
            memcpy(thispkt->data, in, inl);
            thispkt->len = inl;
        }
        thispkt->num = (pktnum >= 0) ? (unsigned int)pktnum : ctx->lastpkt + i;
        thispkt->type = type;
    }

    for (i = 0; i < sk_MEMPACKET_num(ctx->pkts); i++) {
        if (!TEST_ptr(looppkt = sk_MEMPACKET_value(ctx->pkts, i)))
            goto err;
        /* Check if we found the right place to insert this packet */
        if (looppkt->num > thispkt->num) {
            if (sk_MEMPACKET_insert(ctx->pkts, thispkt, i) == 0)
                goto err;
            /* If we're doing up front injection then we're done */
            if (pktnum >= 0)
                return inl;
            /*
             * We need to do some accounting on lastpkt. We increment it first,
             * but it might now equal the value of injected packets, so we need
             * to skip over those
             */
            ctx->lastpkt++;
            do {
                i++;
                nextpkt = sk_MEMPACKET_value(ctx->pkts, i);
                if (nextpkt != NULL && nextpkt->num == ctx->lastpkt)
                    ctx->lastpkt++;
                else
                    return inl;
            } while(1);
        } else if (looppkt->num == thispkt->num) {
            if (!ctx->noinject) {
                /* We injected two packets with the same packet number! */
                goto err;
            }
            ctx->lastpkt++;
            thispkt->num++;
        }
    }
    /*
     * We didn't find any packets with a packet number equal to or greater than
     * this one, so we just add it onto the end
     */
    for (i = 0; i < (duprec ? 3 : 1); i++) {
        thispkt = allpkts[i];
        if (!sk_MEMPACKET_push(ctx->pkts, thispkt))
            goto err;

        if (pktnum < 0)
            ctx->lastpkt++;
    }

    return inl;

 err:
    for (i = 0; i < (ctx->duprec > 0 ? 3 : 1); i++)
        mempacket_free(allpkts[i]);
    return -1;
}