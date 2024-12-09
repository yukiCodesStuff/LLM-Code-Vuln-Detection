    {
        ASN1_OBJECT *asn1obj = OBJ_nid2obj(nid);

        if (asn1obj == NULL || OBJ_length(asn1obj) == 0) {
            ERR_raise(ERR_LIB_EC, EC_R_MISSING_OID);
            return 0;
        }
    if (!eckey_param2type(&ptype, &pval, ec_key)) {
        ERR_raise(ERR_LIB_EC, ERR_R_EC_LIB);
        return 0;
    }
    penclen = i2o_ECPublicKey(ec_key, NULL);
    if (penclen <= 0)
        goto err;
    penc = OPENSSL_malloc(penclen);
    if (penc == NULL)
        goto err;
    p = penc;
    penclen = i2o_ECPublicKey(ec_key, &p);
    if (penclen <= 0)
        goto err;
    if (X509_PUBKEY_set0_param(pk, OBJ_nid2obj(EVP_PKEY_EC),
                               ptype, pval, penc, penclen))
        return 1;
 err:
    if (ptype == V_ASN1_SEQUENCE)
        ASN1_STRING_free(pval);
    OPENSSL_free(penc);
    return 0;
}

static int eckey_pub_decode(EVP_PKEY *pkey, const X509_PUBKEY *pubkey)
{
    const unsigned char *p = NULL;
    int pklen;
    EC_KEY *eckey = NULL;
    X509_ALGOR *palg;
    OSSL_LIB_CTX *libctx = NULL;
    const char *propq = NULL;

    if (!ossl_x509_PUBKEY_get0_libctx(&libctx, &propq, pubkey)
        || !X509_PUBKEY_get0_param(NULL, &p, &pklen, &palg, pubkey))
        return 0;
    eckey = ossl_ec_key_param_from_x509_algor(palg, libctx, propq);

    if (!eckey)
        return 0;

    /* We have parameters now set public key */
    if (!o2i_ECPublicKey(&eckey, &p, pklen)) {
        ERR_raise(ERR_LIB_EC, EC_R_DECODE_ERROR);
        goto ecerr;
    }
    if (eplen <= 0) {
        ERR_raise(ERR_LIB_EC, ERR_R_EC_LIB);
        goto err;
    }