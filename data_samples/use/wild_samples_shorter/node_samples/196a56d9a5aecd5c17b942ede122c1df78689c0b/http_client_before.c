    char *proxy;                /* Optional proxy name or URI */
    char *server;               /* Optional server host name */
    char *port;                 /* Optional server port */
    BIO *mem;                   /* Memory BIO holding request/response header */
    BIO *req;                   /* BIO holding the request provided by caller */
    int method_POST;            /* HTTP method is POST (else GET) */
    char *expected_ct;          /* Optional expected Content-Type */
    int expect_asn1;            /* Response must be ASN.1-encoded */
static int set1_content(OSSL_HTTP_REQ_CTX *rctx,
                        const char *content_type, BIO *req)
{
    long req_len;

    if (rctx == NULL || (req == NULL && content_type != NULL)) {
        ERR_raise(ERR_LIB_HTTP, ERR_R_PASSED_NULL_PARAMETER);
        return 0;
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

int OSSL_HTTP_REQ_CTX_set1_req(OSSL_HTTP_REQ_CTX *rctx, const char *content_type,
                               const ASN1_ITEM *it, const ASN1_VALUE *req)
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