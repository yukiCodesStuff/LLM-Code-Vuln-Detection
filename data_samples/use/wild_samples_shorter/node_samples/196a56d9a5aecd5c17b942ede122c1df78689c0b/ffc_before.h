/*
 * Copyright 2019-2021 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
     */
    const char *mdname;
    const char *mdprops;
} FFC_PARAMS;

void ossl_ffc_params_init(FFC_PARAMS *params);
void ossl_ffc_params_cleanup(FFC_PARAMS *params);
int ossl_ffc_named_group_get_uid(const DH_NAMED_GROUP *group);
const char *ossl_ffc_named_group_get_name(const DH_NAMED_GROUP *);
#ifndef OPENSSL_NO_DH
const BIGNUM *ossl_ffc_named_group_get_q(const DH_NAMED_GROUP *group);
int ossl_ffc_named_group_set_pqg(FFC_PARAMS *ffc, const DH_NAMED_GROUP *group);
#endif

#endif /* OSSL_INTERNAL_FFC_H */