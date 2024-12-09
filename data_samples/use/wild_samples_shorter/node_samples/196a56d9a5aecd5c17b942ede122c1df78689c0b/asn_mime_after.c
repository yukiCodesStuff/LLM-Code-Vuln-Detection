/*
 * Copyright 2008-2022 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
int i2d_ASN1_bio_stream(BIO *out, ASN1_VALUE *val, BIO *in, int flags,
                        const ASN1_ITEM *it)
{
    int rv = 1;

    /* If streaming create stream BIO and copy all content through it */
    if (flags & SMIME_STREAM) {
        BIO *bio, *tbio;
        bio = BIO_new_NDEF(out, val, it);
            ERR_raise(ERR_LIB_ASN1, ERR_R_MALLOC_FAILURE);
            return 0;
        }
        if (!SMIME_crlf_copy(in, bio, flags)) {
            rv = 0;
        }

        (void)BIO_flush(bio);
        /* Free up successive BIOs until we hit the old output BIO */
        do {
            tbio = BIO_pop(bio);
     */
    else
        ASN1_item_i2d_bio(it, out, val);
    return rv;
}

/* Base 64 read and write of ASN1 structure */

     * set up to finalise when it is written through.
     */
    if (!(flags & SMIME_DETACHED) || (flags & PKCS7_REUSE_DIGEST)) {
        return SMIME_crlf_copy(data, out, flags);
    }

    if (!aux || !aux->asn1_cb) {
        ERR_raise(ERR_LIB_ASN1, ASN1_R_STREAMING_NOT_SUPPORTED);
        return 0;

    /* Copy data across, passing through filter BIOs for processing */
    if (!SMIME_crlf_copy(data, sarg.ndef_bio, flags))
        rv = 0;

    /* Finalize structure */
    if (aux->asn1_cb(ASN1_OP_DETACHED_POST, &val, it, &sarg) <= 0)
        rv = 0;
     * when streaming as we don't end up with one OCTET STRING per line.
     */
    bf = BIO_new(BIO_f_buffer());
    if (bf == NULL) {
        ERR_raise(ERR_LIB_ASN1, ERR_R_MALLOC_FAILURE);
        return 0;
    }
    out = BIO_push(bf, out);
    if (flags & SMIME_BINARY) {
        while ((len = BIO_read(in, linebuf, MAX_SMLEN)) > 0)
            BIO_write(out, linebuf, len);