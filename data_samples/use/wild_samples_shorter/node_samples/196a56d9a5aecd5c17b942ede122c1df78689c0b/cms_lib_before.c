/*
 * Copyright 2008-2021 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
                                          (CMS_ContentInfo_it()),
                                          ossl_cms_ctx_get0_libctx(ctx),
                                          ossl_cms_ctx_get0_propq(ctx));
    if (ci != NULL)
        ossl_cms_resolve_libctx(ci);
    return ci;
}

int i2d_CMS_ContentInfo(const CMS_ContentInfo *a, unsigned char **out)