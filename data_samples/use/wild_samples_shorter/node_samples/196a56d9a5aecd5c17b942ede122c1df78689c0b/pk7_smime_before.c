        return 0;
    }

    SMIME_crlf_copy(data, p7bio, flags);

    (void)BIO_flush(p7bio);

    if (!PKCS7_dataFinal(p7, p7bio)) {
                    ERR_raise(ERR_LIB_PKCS7, ERR_R_X509_LIB);
                    goto err;
                }
                X509_STORE_CTX_set_default(cert_ctx, "smime_sign");
            } else if (!X509_STORE_CTX_init(cert_ctx, store, signer, NULL)) {
                ERR_raise(ERR_LIB_PKCS7, ERR_R_X509_LIB);
                goto err;
            }