        ASN1_OBJECT *asn1obj = OBJ_nid2obj(nid);

        if (asn1obj == NULL || OBJ_length(asn1obj) == 0) {
            ASN1_OBJECT_free(asn1obj);
            ERR_raise(ERR_LIB_EC, EC_R_MISSING_OID);
            return 0;
        }
        *ppval = asn1obj;
                               ptype, pval, penc, penclen))
        return 1;
 err:
    if (ptype == V_ASN1_OBJECT)
        ASN1_OBJECT_free(pval);
    else
        ASN1_STRING_free(pval);
    OPENSSL_free(penc);
    return 0;
}
    eplen = i2d_ECPrivateKey(&ec_key, &ep);
    if (eplen <= 0) {
        ERR_raise(ERR_LIB_EC, ERR_R_EC_LIB);
        ASN1_STRING_free(pval);
        return 0;
    }

    if (!PKCS8_pkey_set0(p8, OBJ_nid2obj(NID_X9_62_id_ecPublicKey), 0,
                         ptype, pval, ep, eplen)) {
        ERR_raise(ERR_LIB_EC, ERR_R_EC_LIB);
        ASN1_STRING_free(pval);
        OPENSSL_clear_free(ep, eplen);
        return 0;
    }

    return 1;
}

static int int_ec_size(const EVP_PKEY *pkey)
{