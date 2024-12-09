    char *proxy;                /* Optional proxy name or URI */
    char *server;               /* Optional server host name */
    char *port;                 /* Optional server port */
    BIO *mem;                   /* Mem BIO holding request header or response */
    BIO *req;                   /* BIO holding the request provided by caller */
    int method_POST;            /* HTTP method is POST (else GET) */
    char *expected_ct;          /* Optional expected Content-Type */
    int expect_asn1;            /* Response must be ASN.1-encoded */
static int set1_content(OSSL_HTTP_REQ_CTX *rctx,
                        const char *content_type, BIO *req)
{
    long req_len = 0;
#ifndef OPENSSL_NO_STDIO
    FILE *fp = NULL;
#endif

    if (rctx == NULL || (req == NULL && content_type != NULL)) {
        ERR_raise(ERR_LIB_HTTP, ERR_R_PASSED_NULL_PARAMETER);
        return 0;
            && BIO_printf(rctx->mem, "Content-Type: %s\r\n", content_type) <= 0)
        return 0;

    /*
     * BIO_CTRL_INFO yields the data length at least for memory BIOs, but for
     * file-based BIOs it gives the current position, which is not what we need.
     */
    if (BIO_method_type(req) == BIO_TYPE_FILE) {
#ifndef OPENSSL_NO_STDIO
        if (BIO_get_fp(req, &fp) == 1 && fseek(fp, 0, SEEK_END) == 0) {
            req_len = ftell(fp);
            (void)fseek(fp, 0, SEEK_SET);
        } else {
            fp = NULL;
        }
#endif
    } else {
        req_len = BIO_ctrl(req, BIO_CTRL_INFO, 0, NULL);
        /*
         * Streaming BIOs likely will not support querying the size at all,
         * and we assume we got a correct value if req_len > 0.
         */
    }
    if ((
#ifndef OPENSSL_NO_STDIO
         fp != NULL /* definitely correct req_len */ ||
#endif
         req_len > 0)
            && BIO_printf(rctx->mem, "Content-Length: %ld\r\n", req_len) < 0)
        return 0;

    if (!BIO_up_ref(req))
        return 0;
    rctx->req = req;
    return 1;
}

int OSSL_HTTP_REQ_CTX_set1_req(OSSL_HTTP_REQ_CTX *rctx, const char *content_type,
                               const ASN1_ITEM *it, const ASN1_VALUE *req)
        if (rctx->req != NULL && !BIO_eof(rctx->req)) {
            n = BIO_read(rctx->req, rctx->buf, rctx->buf_size);
            if (n <= 0) {
                if (BIO_should_retry(rctx->req))
                    return -1;
                ERR_raise(ERR_LIB_HTTP, HTTP_R_FAILED_READING_DATA);
                return 0;
            }
    if (bio_update_fn != NULL) {
        BIO *orig_bio = cbio;

        cbio = (*bio_update_fn)(cbio, arg, 1 /* connect */, use_ssl != 0);
        if (cbio == NULL) {
            if (bio == NULL) /* cbio was not provided by caller */
                BIO_free_all(orig_bio);
            goto end;