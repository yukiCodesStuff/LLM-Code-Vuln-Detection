/*
 * Copyright 1995-2021 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#include <stdio.h>
#include "internal/cryptlib.h"
#include <openssl/bn.h>
#include <openssl/evp.h>
#include <openssl/asn1.h>
#include <openssl/asn1t.h>
#include <openssl/x509.h>
#include "crypto/x509.h"
#include <openssl/objects.h>
#include <openssl/buffer.h>
#include <openssl/pem.h>

X509_REQ *X509_to_X509_REQ(X509 *x, EVP_PKEY *pkey, const EVP_MD *md)
{
    X509_REQ *ret;
    X509_REQ_INFO *ri;
    int i;
    EVP_PKEY *pktmp;

    ret = X509_REQ_new_ex(x->libctx, x->propq);
    if (ret == NULL) {
        ERR_raise(ERR_LIB_X509, ERR_R_MALLOC_FAILURE);
        goto err;
    }

    ri = &ret->req_info;

    ri->version->length = 1;
    ri->version->data = OPENSSL_malloc(1);
    if (ri->version->data == NULL)
        goto err;
    ri->version->data[0] = 0;   /* version == 0 */

    if (!X509_REQ_set_subject_name(ret, X509_get_subject_name(x)))
        goto err;

    pktmp = X509_get0_pubkey(x);
    if (pktmp == NULL)
        goto err;
    i = X509_REQ_set_pubkey(ret, pktmp);
    if (!i)
        goto err;

    if (pkey != NULL) {
        if (!X509_REQ_sign(ret, pkey, md))
            goto err;
    }
    return ret;
 err:
    X509_REQ_free(ret);
    return NULL;
}
{
    int i, nid;
    for (i = 0;; i++) {
        nid = ext_nids[i];
        if (nid == NID_undef)
            return 0;
        else if (req_nid == nid)
            return 1;
    }
}
{
    X509_ATTRIBUTE *attr;
    ASN1_TYPE *ext = NULL;
    int idx, *pnid;
    const unsigned char *p;

    if ((req == NULL) || !ext_nids)
        return NULL;
    for (pnid = ext_nids; *pnid != NID_undef; pnid++) {
        idx = X509_REQ_get_attr_by_NID(req, *pnid, -1);
        if (idx == -1)
            continue;
        attr = X509_REQ_get_attr(req, idx);
        ext = X509_ATTRIBUTE_get0_type(attr, 0);
        break;
    }
{
    return X509at_get_attr(req->req_info.attributes, loc);
}

X509_ATTRIBUTE *X509_REQ_delete_attr(X509_REQ *req, int loc)
{
    return X509at_delete_attr(req->req_info.attributes, loc);
}
{
    if (req->signature)
           ASN1_BIT_STRING_free(req->signature);
    req->signature = psig;
}
{
    return OBJ_obj2nid(req->sig_alg.algorithm);
}

int i2d_re_X509_REQ_tbs(X509_REQ *req, unsigned char **pp)
{
    req->req_info.enc.modified = 1;
    return i2d_X509_REQ_INFO(&req->req_info, pp);
}