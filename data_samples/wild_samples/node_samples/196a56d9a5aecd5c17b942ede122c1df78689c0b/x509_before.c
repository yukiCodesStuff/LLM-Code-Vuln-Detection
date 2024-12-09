            if ((objtmp = OBJ_txt2obj(opt_arg(), 0)) == NULL) {
                BIO_printf(bio_err, "%s: Invalid reject object value %s\n",
                           prog, opt_arg());
                goto opthelp;
            }
            sk_ASN1_OBJECT_push(reject, objtmp);
            trustout = 1;
            break;
        case OPT_SETALIAS:
            alias = opt_arg();
            trustout = 1;
            break;
        case OPT_CERTOPT:
            if (!set_cert_ex(&certflag, opt_arg()))
                goto opthelp;
            break;
        case OPT_NAMEOPT:
            if (!set_nameopt(opt_arg()))
                goto opthelp;
            break;
        case OPT_ENGINE:
            e = setup_engine(opt_arg(), 0);
            break;
        case OPT_EMAIL:
            email = ++num;
            break;
        case OPT_OCSP_URI:
            ocsp_uri = ++num;
            break;
        case OPT_SERIAL:
            serial = ++num;
            break;
        case OPT_NEXT_SERIAL:
            next_serial = ++num;
            break;
        case OPT_MODULUS:
            modulus = ++num;
            break;
        case OPT_PUBKEY:
            print_pubkey = ++num;
            break;
        case OPT_X509TOREQ:
            x509toreq = 1;
            break;
        case OPT_TEXT:
            text = ++num;
            break;
        case OPT_SUBJECT:
            subject = ++num;
            break;
        case OPT_ISSUER:
            issuer = ++num;
            break;
        case OPT_FINGERPRINT:
            fingerprint = ++num;
            break;
        case OPT_HASH:
            subject_hash = ++num;
            break;
        case OPT_ISSUER_HASH:
            issuer_hash = ++num;
            break;
        case OPT_PURPOSE:
            pprint = ++num;
            break;
        case OPT_STARTDATE:
            startdate = ++num;
            break;
        case OPT_ENDDATE:
            enddate = ++num;
            break;
        case OPT_NOOUT:
            noout = ++num;
            break;
        case OPT_EXT:
            ext = ++num;
            ext_names = opt_arg();
            break;
        case OPT_NOCERT:
            nocert = 1;
            break;
        case OPT_TRUSTOUT:
            trustout = 1;
            break;
        case OPT_CLRTRUST:
            clrtrust = ++num;
            break;
        case OPT_CLRREJECT:
            clrreject = ++num;
            break;
        case OPT_ALIAS:
            aliasout = ++num;
            break;
        case OPT_CACREATESERIAL:
            CA_createserial = ++num;
            break;
        case OPT_CLREXT:
            clrext = 1;
            break;
        case OPT_OCSPID:
            ocspid = ++num;
            break;
        case OPT_BADSIG:
            badsig = 1;
            break;
#ifndef OPENSSL_NO_MD5
        case OPT_SUBJECT_HASH_OLD:
            subject_hash_old = ++num;
            break;
        case OPT_ISSUER_HASH_OLD:
            issuer_hash_old = ++num;
            break;
#else
        case OPT_SUBJECT_HASH_OLD:
        case OPT_ISSUER_HASH_OLD:
            break;
#endif
        case OPT_DATES:
            startdate = ++num;
            enddate = ++num;
            break;
        case OPT_CHECKEND:
            checkend = 1;
            {
                ossl_intmax_t temp = 0;
                if (!opt_intmax(opt_arg(), &temp))
                    goto opthelp;
                checkoffset = (time_t)temp;
                if ((ossl_intmax_t)checkoffset != temp) {
                    BIO_printf(bio_err, "%s: Checkend time out of range %s\n",
                               prog, opt_arg());
                    goto opthelp;
                }
            }
        if (privkeyfile != NULL) {
            BIO_printf(bio_err, "Cannot use both -key/-signkey and -CA option\n");
            goto end;
        }
    } else if (CAkeyfile != NULL) {
        BIO_printf(bio_err,
                   "Warning: ignoring -CAkey option since no -CA option is given\n");
    }

    if (extfile == NULL) {
        if (extsect != NULL)
            BIO_printf(bio_err,
                       "Warning: ignoring -extensions option without -extfile\n");
    } else {
        X509V3_CTX ctx2;

        if ((extconf = app_load_config(extfile)) == NULL)
            goto end;
        if (extsect == NULL) {
            extsect = NCONF_get_string(extconf, "default", "extensions");
            if (extsect == NULL) {
                ERR_clear_error();
                extsect = "default";
            }
        }
        X509V3_set_ctx_test(&ctx2);
        X509V3_set_nconf(&ctx2, extconf);
        if (!X509V3_EXT_add_nconf(extconf, &ctx2, extsect, NULL)) {
            BIO_printf(bio_err,
                       "Error checking extension section %s\n", extsect);
            goto end;
        }
    }
        if (privkeyfile == NULL && CAkeyfile == NULL) {
            BIO_printf(bio_err,
                       "We need a private key to sign with, use -key or -CAkey or -CA with private key\n");
            goto end;
        }
        if ((x = X509_new_ex(app_get0_libctx(), app_get0_propq())) == NULL)
            goto end;
        if (sno == NULL) {
            sno = ASN1_INTEGER_new();
            if (sno == NULL || !rand_serial(NULL, sno))
                goto end;
        }
{
    char *buf = NULL;
    ASN1_INTEGER *bs = NULL;
    BIGNUM *serial = NULL;

    if (serialfile == NULL) {
        const char *p = strrchr(CAfile, '.');
        size_t len = p != NULL ? (size_t)(p - CAfile) : strlen(CAfile);

        buf = app_malloc(len + sizeof(POSTFIX), "serial# buffer");
        memcpy(buf, CAfile, len);
        memcpy(buf + len, POSTFIX, sizeof(POSTFIX));
        serialfile = buf;
    }
        size_t len = p != NULL ? (size_t)(p - CAfile) : strlen(CAfile);

        buf = app_malloc(len + sizeof(POSTFIX), "serial# buffer");
        memcpy(buf, CAfile, len);
        memcpy(buf + len, POSTFIX, sizeof(POSTFIX));
        serialfile = buf;
    }

    serial = load_serial(serialfile, create, NULL);
    if (serial == NULL)
        goto end;

    if (!BN_add_word(serial, 1)) {
        BIO_printf(bio_err, "Serial number increment failure\n");
        goto end;
    }

    if (!save_serial(serialfile, NULL, serial, &bs))
        goto end;

 end:
    OPENSSL_free(buf);
    BN_free(serial);
    return bs;
}

static int callb(int ok, X509_STORE_CTX *ctx)
{
    int err;
    X509 *err_cert;

    /*
     * It is ok to use a self-signed certificate. This case will catch both
     * the initial ok == 0 and the final ok == 1 calls to this function.
     */
    err = X509_STORE_CTX_get_error(ctx);
    if (err == X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT)
        return 1;

    /*
     * BAD we should have gotten an error.  Normally if everything worked
     * X509_STORE_CTX_get_error(ctx) will still be set to
     * DEPTH_ZERO_SELF_....
     */
    if (ok) {
        BIO_printf(bio_err,
                   "Error with certificate to be certified - should be self-signed\n");
        return 0;
    } else {
        err_cert = X509_STORE_CTX_get_current_cert(ctx);
        print_name(bio_err, "subject=", X509_get_subject_name(err_cert));
        BIO_printf(bio_err,
                   "Error with certificate - error %d at depth %d\n%s\n", err,
                   X509_STORE_CTX_get_error_depth(ctx),
                   X509_verify_cert_error_string(err));
        return 1;
    }
    if (!BN_add_word(serial, 1)) {
        BIO_printf(bio_err, "Serial number increment failure\n");
        goto end;
    }

    if (!save_serial(serialfile, NULL, serial, &bs))
        goto end;

 end:
    OPENSSL_free(buf);
    BN_free(serial);
    return bs;
}

static int callb(int ok, X509_STORE_CTX *ctx)
{
    int err;
    X509 *err_cert;

    /*
     * It is ok to use a self-signed certificate. This case will catch both
     * the initial ok == 0 and the final ok == 1 calls to this function.
     */
    err = X509_STORE_CTX_get_error(ctx);
    if (err == X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT)
        return 1;

    /*
     * BAD we should have gotten an error.  Normally if everything worked
     * X509_STORE_CTX_get_error(ctx) will still be set to
     * DEPTH_ZERO_SELF_....
     */
    if (ok) {
        BIO_printf(bio_err,
                   "Error with certificate to be certified - should be self-signed\n");
        return 0;
    } else {
        err_cert = X509_STORE_CTX_get_current_cert(ctx);
        print_name(bio_err, "subject=", X509_get_subject_name(err_cert));
        BIO_printf(bio_err,
                   "Error with certificate - error %d at depth %d\n%s\n", err,
                   X509_STORE_CTX_get_error_depth(ctx),
                   X509_verify_cert_error_string(err));
        return 1;
    }