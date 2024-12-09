/*
 * Copyright 2008-2022 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#include <openssl/asn1t.h>
#include <openssl/x509v3.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/bio.h>
#include <openssl/asn1.h>
#include <openssl/cms.h>
#include <openssl/cms.h>
#include "internal/sizes.h"
#include "crypto/x509.h"
#include "cms_local.h"

static STACK_OF(CMS_CertificateChoices)
**cms_get0_certificate_choices(CMS_ContentInfo *cms);

IMPLEMENT_ASN1_PRINT_FUNCTION(CMS_ContentInfo)

CMS_ContentInfo *d2i_CMS_ContentInfo(CMS_ContentInfo **a,
                                     const unsigned char **in, long len)
{
    CMS_ContentInfo *ci;
    const CMS_CTX *ctx = ossl_cms_get0_cmsctx(a == NULL ? NULL : *a);

    ci = (CMS_ContentInfo *)ASN1_item_d2i_ex((ASN1_VALUE **)a, in, len,
                                          (CMS_ContentInfo_it()),
                                          ossl_cms_ctx_get0_libctx(ctx),
                                          ossl_cms_ctx_get0_propq(ctx));
    if (ci != NULL) {
        ERR_set_mark();
        ossl_cms_resolve_libctx(ci);
        ERR_pop_to_mark();
    }
    return ci;
}
{
    CMS_ContentInfo *ci;
    const CMS_CTX *ctx = ossl_cms_get0_cmsctx(a == NULL ? NULL : *a);

    ci = (CMS_ContentInfo *)ASN1_item_d2i_ex((ASN1_VALUE **)a, in, len,
                                          (CMS_ContentInfo_it()),
                                          ossl_cms_ctx_get0_libctx(ctx),
                                          ossl_cms_ctx_get0_propq(ctx));
    if (ci != NULL) {
        ERR_set_mark();
        ossl_cms_resolve_libctx(ci);
        ERR_pop_to_mark();
    }