    if ((p7bio = PKCS7_dataInit(p7, NULL)) == NULL) {
        ERR_raise(ERR_LIB_PKCS7, ERR_R_MALLOC_FAILURE);
        return 0;
    }

    if (!SMIME_crlf_copy(data, p7bio, flags))
        goto err;

    (void)BIO_flush(p7bio);

    if (!PKCS7_dataFinal(p7, p7bio)) {
        ERR_raise(ERR_LIB_PKCS7, PKCS7_R_PKCS7_DATASIGN);
        goto err;
    }
    ret = 1;
err:
    BIO_free_all(p7bio);

    return ret;

}

/* Check to see if a cipher exists and if so add S/MIME capabilities */

static int add_cipher_smcap(STACK_OF(X509_ALGOR) *sk, int nid, int arg)
{
    if (EVP_get_cipherbynid(nid))
        return PKCS7_simple_smimecap(sk, nid, arg);
    return 1;
}

static int add_digest_smcap(STACK_OF(X509_ALGOR) *sk, int nid, int arg)
{
    if (EVP_get_digestbynid(nid))
        return PKCS7_simple_smimecap(sk, nid, arg);
    return 1;
}

PKCS7_SIGNER_INFO *PKCS7_sign_add_signer(PKCS7 *p7, X509 *signcert,
                                         EVP_PKEY *pkey, const EVP_MD *md,
                                         int flags)
{
    PKCS7_SIGNER_INFO *si = NULL;
    STACK_OF(X509_ALGOR) *smcap = NULL;

    if (!X509_check_private_key(signcert, pkey)) {
        ERR_raise(ERR_LIB_PKCS7,
                  PKCS7_R_PRIVATE_KEY_DOES_NOT_MATCH_CERTIFICATE);
        return NULL;
    }

    if ((si = PKCS7_add_signature(p7, signcert, pkey, md)) == NULL) {
        ERR_raise(ERR_LIB_PKCS7, PKCS7_R_PKCS7_ADD_SIGNATURE_ERROR);
        return NULL;
    }

    si->ctx = ossl_pkcs7_get0_ctx(p7);
    if (!(flags & PKCS7_NOCERTS)) {
        if (!PKCS7_add_certificate(p7, signcert))
            goto err;
    }

    if (!(flags & PKCS7_NOATTR)) {
        if (!PKCS7_add_attrib_content_type(si, NULL))
            goto err;
        /* Add SMIMECapabilities */
        if (!(flags & PKCS7_NOSMIMECAP)) {
            if ((smcap = sk_X509_ALGOR_new_null()) == NULL) {
                ERR_raise(ERR_LIB_PKCS7, ERR_R_MALLOC_FAILURE);
                goto err;
            }
            if (!add_cipher_smcap(smcap, NID_aes_256_cbc, -1)
                || !add_digest_smcap(smcap, NID_id_GostR3411_2012_256, -1)
                || !add_digest_smcap(smcap, NID_id_GostR3411_2012_512, -1)
                || !add_digest_smcap(smcap, NID_id_GostR3411_94, -1)
                || !add_cipher_smcap(smcap, NID_id_Gost28147_89, -1)
                || !add_cipher_smcap(smcap, NID_aes_192_cbc, -1)
                || !add_cipher_smcap(smcap, NID_aes_128_cbc, -1)
                || !add_cipher_smcap(smcap, NID_des_ede3_cbc, -1)
                || !add_cipher_smcap(smcap, NID_rc2_cbc, 128)
                || !add_cipher_smcap(smcap, NID_rc2_cbc, 64)
                || !add_cipher_smcap(smcap, NID_des_cbc, -1)
                || !add_cipher_smcap(smcap, NID_rc2_cbc, 40)
                || !PKCS7_add_attrib_smimecap(si, smcap))
                goto err;
            sk_X509_ALGOR_pop_free(smcap, X509_ALGOR_free);
            smcap = NULL;
        }
                                         p7->d.sign->cert)) {
                    ERR_raise(ERR_LIB_PKCS7, ERR_R_X509_LIB);
                    goto err;
                }
                if (!X509_STORE_CTX_set_default(cert_ctx, "smime_sign"))
                    goto err;
            } else if (!X509_STORE_CTX_init(cert_ctx, store, signer, NULL)) {
                ERR_raise(ERR_LIB_PKCS7, ERR_R_X509_LIB);
                goto err;
            }
            if (!(flags & PKCS7_NOCRL))
                X509_STORE_CTX_set0_crls(cert_ctx, p7->d.sign->crl);
            i = X509_verify_cert(cert_ctx);
            if (i <= 0)
                j = X509_STORE_CTX_get_error(cert_ctx);
            if (i <= 0) {
                ERR_raise_data(ERR_LIB_PKCS7, PKCS7_R_CERTIFICATE_VERIFY_ERROR,
                               "Verify error: %s",
                               X509_verify_cert_error_string(j));
                goto err;
            }
            /* Check for revocation status here */
        }

    /*
     * Performance optimization: if the content is a memory BIO then store
     * its contents in a temporary read only memory BIO. This avoids
     * potentially large numbers of slow copies of data which will occur when
     * reading from a read write memory BIO when signatures are calculated.
     */

    if (indata && (BIO_method_type(indata) == BIO_TYPE_MEM)) {
        char *ptr;
        long len;
        len = BIO_get_mem_data(indata, &ptr);
        tmpin = (len == 0) ? indata : BIO_new_mem_buf(ptr, len);
        if (tmpin == NULL) {
            ERR_raise(ERR_LIB_PKCS7, ERR_R_MALLOC_FAILURE);
            goto err;
        }
    } else
        tmpin = indata;

    if ((p7bio = PKCS7_dataInit(p7, tmpin)) == NULL)
        goto err;

    if (flags & PKCS7_TEXT) {
        if ((tmpout = BIO_new(BIO_s_mem())) == NULL) {
            ERR_raise(ERR_LIB_PKCS7, ERR_R_MALLOC_FAILURE);
            goto err;
        }