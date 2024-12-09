/*
 * Copyright 1995-2021 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
int X509_REQ_extension_nid(int req_nid)
{
    int i, nid;
    for (i = 0;; i++) {
        nid = ext_nids[i];
        if (nid == NID_undef)
            return 0;
    int idx, *pnid;
    const unsigned char *p;

    if ((req == NULL) || !ext_nids)
        return NULL;
    for (pnid = ext_nids; *pnid != NID_undef; pnid++) {
        idx = X509_REQ_get_attr_by_NID(req, *pnid, -1);
        if (idx == -1)

X509_ATTRIBUTE *X509_REQ_delete_attr(X509_REQ *req, int loc)
{
    return X509at_delete_attr(req->req_info.attributes, loc);
}

int X509_REQ_add1_attr(X509_REQ *req, X509_ATTRIBUTE *attr)
{
    if (X509at_add1_attr(&req->req_info.attributes, attr))
        return 1;
    return 0;
}

int X509_REQ_add1_attr_by_OBJ(X509_REQ *req,
                              const ASN1_OBJECT *obj, int type,
                              const unsigned char *bytes, int len)
{
    if (X509at_add1_attr_by_OBJ(&req->req_info.attributes, obj,
                                type, bytes, len))
        return 1;
    return 0;
}

int X509_REQ_add1_attr_by_NID(X509_REQ *req,
                              int nid, int type,
                              const unsigned char *bytes, int len)
{
    if (X509at_add1_attr_by_NID(&req->req_info.attributes, nid,
                                type, bytes, len))
        return 1;
    return 0;
}

int X509_REQ_add1_attr_by_txt(X509_REQ *req,
                              const char *attrname, int type,
                              const unsigned char *bytes, int len)
{
    if (X509at_add1_attr_by_txt(&req->req_info.attributes, attrname,
                                type, bytes, len))
        return 1;
    return 0;
}

long X509_REQ_get_version(const X509_REQ *req)
{
void X509_REQ_set0_signature(X509_REQ *req, ASN1_BIT_STRING *psig)
{
    if (req->signature)
           ASN1_BIT_STRING_free(req->signature);
    req->signature = psig;
}

int X509_REQ_set1_signature_algo(X509_REQ *req, X509_ALGOR *palg)

int i2d_re_X509_REQ_tbs(X509_REQ *req, unsigned char **pp)
{
    req->req_info.enc.modified = 1;
    return i2d_X509_REQ_INFO(&req->req_info, pp);
}