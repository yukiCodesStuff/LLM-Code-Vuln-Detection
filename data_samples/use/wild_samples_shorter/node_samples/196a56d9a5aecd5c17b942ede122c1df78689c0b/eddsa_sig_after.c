/*
 * Copyright 2020-2022 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
        return 0;
    }
#ifdef S390X_EC_ASM
    if (S390X_CAN_SIGN(ED25519)) {
	    if (s390x_ed25519_digestsign(edkey, sigret, tbs, tbslen) == 0) {
		    ERR_raise(ERR_LIB_PROV, PROV_R_FAILED_TO_SIGN);
		    return 0;
	    }
	    *siglen = ED25519_SIGSIZE;
	    return 1;
    }
#endif /* S390X_EC_ASM */
    if (ossl_ed25519_sign(sigret, tbs, tbslen, edkey->pubkey, edkey->privkey,
                          peddsactx->libctx, NULL) == 0) {
        ERR_raise(ERR_LIB_PROV, PROV_R_FAILED_TO_SIGN);
        return 0;
    }
#ifdef S390X_EC_ASM
    if (S390X_CAN_SIGN(ED448)) {
        if (s390x_ed448_digestsign(edkey, sigret, tbs, tbslen) == 0) {
		ERR_raise(ERR_LIB_PROV, PROV_R_FAILED_TO_SIGN);
		return 0;
	}
	*siglen = ED448_SIGSIZE;
	return 1;
    }
#endif /* S390X_EC_ASM */
    if (ossl_ed448_sign(peddsactx->libctx, sigret, tbs, tbslen, edkey->pubkey,
                        edkey->privkey, NULL, 0, edkey->propq) == 0) {
        ERR_raise(ERR_LIB_PROV, PROV_R_FAILED_TO_SIGN);