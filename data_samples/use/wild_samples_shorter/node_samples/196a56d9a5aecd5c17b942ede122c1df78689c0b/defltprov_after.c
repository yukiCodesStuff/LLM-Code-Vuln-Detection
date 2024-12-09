/*
 * Copyright 2019-2022 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
    { PROV_NAMES_MD5_SHA1, "provider=default", ossl_md5_sha1_functions },
#endif /* OPENSSL_NO_MD5 */

#ifndef OPENSSL_NO_RMD160
    { PROV_NAMES_RIPEMD_160, "provider=default", ossl_ripemd160_functions },
#endif /* OPENSSL_NO_RMD160 */

    { PROV_NAMES_NULL, "provider=default", ossl_nullmd_functions },
    { NULL, NULL, NULL }
};
