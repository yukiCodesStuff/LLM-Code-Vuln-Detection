/*
 * Copyright 1995-2022 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at

int X509_verify(X509 *a, EVP_PKEY *r)
{
    if (X509_ALGOR_cmp(&a->sig_alg, &a->cert_info.signature) != 0)
        return 0;

    return ASN1_item_verify_ex(ASN1_ITEM_rptr(X509_CINF), &a->sig_alg,
                               &a->signature, &a->cert_info,

int X509_sign(X509 *x, EVP_PKEY *pkey, const EVP_MD *md)
{
    if (x == NULL) {
        ERR_raise(ERR_LIB_X509, ERR_R_PASSED_NULL_PARAMETER);
        return 0;
    }

    /*
     * Setting the modified flag before signing it. This makes the cached
     * encoding to be ignored, so even if the certificate fields have changed,
     * they are signed correctly.
     * The X509_sign_ctx, X509_REQ_sign{,_ctx}, X509_CRL_sign{,_ctx} functions
     * which exist below are the same.
     */
    x->cert_info.enc.modified = 1;
    return ASN1_item_sign_ex(ASN1_ITEM_rptr(X509_CINF), &x->cert_info.signature,
                             &x->sig_alg, &x->signature, &x->cert_info, NULL,
                             pkey, md, x->libctx, x->propq);

int X509_sign_ctx(X509 *x, EVP_MD_CTX *ctx)
{
    if (x == NULL) {
        ERR_raise(ERR_LIB_X509, ERR_R_PASSED_NULL_PARAMETER);
        return 0;
    }
    x->cert_info.enc.modified = 1;
    return ASN1_item_sign_ctx(ASN1_ITEM_rptr(X509_CINF),
                              &x->cert_info.signature,
                              &x->sig_alg, &x->signature, &x->cert_info, ctx);
                                   int timeout, const ASN1_ITEM *it)
{
    BIO *mem = OSSL_HTTP_get(url, NULL /* proxy */, NULL /* no_proxy */,
                             bio, rbio, NULL /* cb */, NULL /* arg */,
                             1024 /* buf_size */, NULL /* headers */,
                             NULL /* expected_ct */, 1 /* expect_asn1 */,
                             OSSL_HTTP_DEFAULT_MAX_RESP_LEN, timeout);
    ASN1_VALUE *res = ASN1_item_d2i_bio(it, mem, NULL);

int X509_REQ_sign(X509_REQ *x, EVP_PKEY *pkey, const EVP_MD *md)
{
    if (x == NULL) {
        ERR_raise(ERR_LIB_X509, ERR_R_PASSED_NULL_PARAMETER);
        return 0;
    }
    x->req_info.enc.modified = 1;
    return ASN1_item_sign_ex(ASN1_ITEM_rptr(X509_REQ_INFO), &x->sig_alg, NULL,
                             x->signature, &x->req_info, NULL,
                             pkey, md, x->libctx, x->propq);
}

int X509_REQ_sign_ctx(X509_REQ *x, EVP_MD_CTX *ctx)
{
    if (x == NULL) {
        ERR_raise(ERR_LIB_X509, ERR_R_PASSED_NULL_PARAMETER);
        return 0;
    }
    x->req_info.enc.modified = 1;
    return ASN1_item_sign_ctx(ASN1_ITEM_rptr(X509_REQ_INFO),
                              &x->sig_alg, NULL, x->signature, &x->req_info,
                              ctx);
}

int X509_CRL_sign(X509_CRL *x, EVP_PKEY *pkey, const EVP_MD *md)
{
    if (x == NULL) {
        ERR_raise(ERR_LIB_X509, ERR_R_PASSED_NULL_PARAMETER);
        return 0;
    }
    x->crl.enc.modified = 1;
    return ASN1_item_sign_ex(ASN1_ITEM_rptr(X509_CRL_INFO), &x->crl.sig_alg,
                             &x->sig_alg, &x->signature, &x->crl, NULL,
                             pkey, md, x->libctx, x->propq);

int X509_CRL_sign_ctx(X509_CRL *x, EVP_MD_CTX *ctx)
{
    if (x == NULL) {
        ERR_raise(ERR_LIB_X509, ERR_R_PASSED_NULL_PARAMETER);
        return 0;
    }
    x->crl.enc.modified = 1;
    return ASN1_item_sign_ctx(ASN1_ITEM_rptr(X509_CRL_INFO),
                              &x->crl.sig_alg, &x->sig_alg, &x->signature,
                              &x->crl, ctx);

int NETSCAPE_SPKI_sign(NETSCAPE_SPKI *x, EVP_PKEY *pkey, const EVP_MD *md)
{
    return
        ASN1_item_sign_ex(ASN1_ITEM_rptr(NETSCAPE_SPKAC), &x->sig_algor, NULL,
                          x->signature, x->spkac, NULL, pkey, md, NULL, NULL);
}

#ifndef OPENSSL_NO_STDIO
        propq = (*p7)->ctx.propq;
    }

    ret = ASN1_item_d2i_bio_ex(ASN1_ITEM_rptr(PKCS7), bp, p7, libctx, propq);
    if (ret != NULL)
        ossl_pkcs7_resolve_libctx(ret);
    return ret;
int X509_pubkey_digest(const X509 *data, const EVP_MD *type,
                       unsigned char *md, unsigned int *len)
{
    ASN1_BIT_STRING *key = X509_get0_pubkey_bitstr(data);

    if (key == NULL)
        return 0;
    return EVP_Digest(key->data, key->length, md, len, type, NULL);
}

                || !ossl_rsa_pss_get_param_unverified(pss, &mmd, &mgf1md,
                                                      &saltlen,
                                                      &trailerfield)
                || mmd == NULL) {
                RSA_PSS_PARAMS_free(pss);
                ERR_raise(ERR_LIB_X509, X509_R_UNSUPPORTED_ALGORITHM);
                return NULL;
            }
    if (!X509_digest(cert, md, hash, &len)
            || (new = ASN1_OCTET_STRING_new()) == NULL)
        goto err;
    if (ASN1_OCTET_STRING_set(new, hash, len)) {
        if (md_used != NULL)
            *md_used = md;
        else
            EVP_MD_free(md);