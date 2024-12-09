struct ossl_http_req_ctx_st {
    int state;                  /* Current I/O state */
    unsigned char *buf;         /* Buffer to write request or read response */
    int buf_size;               /* Buffer size */
    int free_wbio;              /* wbio allocated internally, free with ctx */
    BIO *wbio;                  /* BIO to write/send request to */
    BIO *rbio;                  /* BIO to read/receive response from */
    OSSL_HTTP_bio_cb_t upd_fn;  /* Optional BIO update callback used for TLS */
    void *upd_arg;              /* Optional arg for update callback function */
    int use_ssl;                /* Use HTTPS */
    char *proxy;                /* Optional proxy name or URI */
    char *server;               /* Optional server host name */
    char *port;                 /* Optional server port */
    BIO *mem;                   /* Memory BIO holding request/response header */
    BIO *req;                   /* BIO holding the request provided by caller */
    int method_POST;            /* HTTP method is POST (else GET) */
    char *expected_ct;          /* Optional expected Content-Type */
    int expect_asn1;            /* Response must be ASN.1-encoded */
    unsigned char *pos;         /* Current position sending data */
    long len_to_send;           /* Number of bytes still to send */
    size_t resp_len;            /* Length of response */
    size_t max_resp_len;        /* Maximum length of response, or 0 */
    int keep_alive;             /* Persistent conn. 0=no, 1=prefer, 2=require */
    time_t max_time;            /* Maximum end time of current transfer, or 0 */
    time_t max_total_time;      /* Maximum end time of total transfer, or 0 */
    char *redirection_url;      /* Location obtained from HTTP status 301/302 */
};

/* HTTP states */

#define OHS_NOREAD         0x1000 /* If set no reading should be performed */
#define OHS_ERROR          (0 | OHS_NOREAD) /* Error condition */
#define OHS_ADD_HEADERS    (1 | OHS_NOREAD) /* Adding header lines to request */
#define OHS_WRITE_INIT     (2 | OHS_NOREAD) /* 1st call: ready to start send */
#define OHS_WRITE_HDR      (3 | OHS_NOREAD) /* Request header being sent */
#define OHS_WRITE_REQ      (4 | OHS_NOREAD) /* Request contents being sent */
#define OHS_FLUSH          (5 | OHS_NOREAD) /* Request being flushed */
#define OHS_FIRSTLINE       1 /* First line of response being read */
#define OHS_HEADERS         2 /* MIME headers of response being read */
#define OHS_REDIRECT        3 /* MIME headers being read, expecting Location */
#define OHS_ASN1_HEADER     4 /* ASN1 sequence header (tag+length) being read */
#define OHS_ASN1_CONTENT    5 /* ASN1 content octets being read */
#define OHS_ASN1_DONE      (6 | OHS_NOREAD) /* ASN1 content read completed */
#define OHS_STREAM         (7 | OHS_NOREAD) /* HTTP content stream to be read */

/* Low-level HTTP API implementation */

OSSL_HTTP_REQ_CTX *OSSL_HTTP_REQ_CTX_new(BIO *wbio, BIO *rbio, int buf_size)
{
    OSSL_HTTP_REQ_CTX *rctx;

    if (wbio == NULL || rbio == NULL) {
        ERR_raise(ERR_LIB_HTTP, ERR_R_PASSED_NULL_PARAMETER);
        return NULL;
    }

    if ((rctx = OPENSSL_zalloc(sizeof(*rctx))) == NULL)
        return NULL;
    rctx->state = OHS_ERROR;
    rctx->buf_size = buf_size > 0 ? buf_size : OSSL_HTTP_DEFAULT_MAX_LINE_LEN;
    rctx->buf = OPENSSL_malloc(rctx->buf_size);
    rctx->wbio = wbio;
    rctx->rbio = rbio;
    if (rctx->buf == NULL) {
        OPENSSL_free(rctx);
        return NULL;
    }
    rctx->max_resp_len = OSSL_HTTP_DEFAULT_MAX_RESP_LEN;
    /* everything else is 0, e.g. rctx->len_to_send, or NULL, e.g. rctx->mem  */
    return rctx;
}
            && rctx->state != OHS_ERROR && rctx->state != OHS_ADD_HEADERS) {
        /* Cannot anymore set keep-alive in request header */
        ERR_raise(ERR_LIB_HTTP, ERR_R_SHOULD_NOT_HAVE_BEEN_CALLED);
        return 0;
    }

    OPENSSL_free(rctx->expected_ct);
    rctx->expected_ct = NULL;
    if (content_type != NULL
            && (rctx->expected_ct = OPENSSL_strdup(content_type)) == NULL)
        return 0;

    rctx->expect_asn1 = asn1;
    if (timeout >= 0)
        rctx->max_time = timeout > 0 ? time(NULL) + timeout : 0;
    else /* take over any |overall_timeout| arg of OSSL_HTTP_open(), else 0 */
        rctx->max_time = rctx->max_total_time;
    rctx->keep_alive = keep_alive;
    return 1;
}

static int set1_content(OSSL_HTTP_REQ_CTX *rctx,
                        const char *content_type, BIO *req)
{
    long req_len;

    if (rctx == NULL || (req == NULL && content_type != NULL)) {
        ERR_raise(ERR_LIB_HTTP, ERR_R_PASSED_NULL_PARAMETER);
        return 0;
    }

    if (rctx->keep_alive != 0
            && !OSSL_HTTP_REQ_CTX_add1_header(rctx, "Connection", "keep-alive"))
        return 0;

    BIO_free(rctx->req);
    rctx->req = NULL;
    if (req == NULL)
        return 1;
    if (!rctx->method_POST) {
        ERR_raise(ERR_LIB_HTTP, ERR_R_SHOULD_NOT_HAVE_BEEN_CALLED);
        return 0;
    }

    if (content_type != NULL
            && BIO_printf(rctx->mem, "Content-Type: %s\r\n", content_type) <= 0)
        return 0;

    /* streaming BIO may not support querying size */
    if (((req_len = BIO_ctrl(req, BIO_CTRL_INFO, 0, NULL)) <= 0
         || BIO_printf(rctx->mem, "Content-Length: %ld\r\n", req_len) > 0)
        && BIO_up_ref(req)) {
        rctx->req = req;
        return 1;
    }
    return 0;
}
    if (!rctx->method_POST) {
        ERR_raise(ERR_LIB_HTTP, ERR_R_SHOULD_NOT_HAVE_BEEN_CALLED);
        return 0;
    }

    if (content_type != NULL
            && BIO_printf(rctx->mem, "Content-Type: %s\r\n", content_type) <= 0)
        return 0;

    /* streaming BIO may not support querying size */
    if (((req_len = BIO_ctrl(req, BIO_CTRL_INFO, 0, NULL)) <= 0
         || BIO_printf(rctx->mem, "Content-Length: %ld\r\n", req_len) > 0)
        && BIO_up_ref(req)) {
        rctx->req = req;
        return 1;
    }
        if (rctx->req != NULL && !BIO_eof(rctx->req)) {
            n = BIO_read(rctx->req, rctx->buf, rctx->buf_size);
            if (n <= 0) {
                if (BIO_should_retry(rctx->rbio))
                    return -1;
                ERR_raise(ERR_LIB_HTTP, HTTP_R_FAILED_READING_DATA);
                return 0;
            }
    if (bio_update_fn != NULL) {
        BIO *orig_bio = cbio;

        cbio = (*bio_update_fn)(cbio, arg, 1 /* connect */, use_ssl);
        if (cbio == NULL) {
            if (bio == NULL) /* cbio was not provided by caller */
                BIO_free_all(orig_bio);
            goto end;
        }