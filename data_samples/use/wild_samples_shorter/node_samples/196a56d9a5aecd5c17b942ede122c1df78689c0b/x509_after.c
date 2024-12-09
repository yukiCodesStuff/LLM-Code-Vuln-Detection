            aliasout = ++num;
            break;
        case OPT_CACREATESERIAL:
            CA_createserial = 1;
            break;
        case OPT_CLREXT:
            clrext = 1;
            break;
            BIO_printf(bio_err, "Cannot use both -key/-signkey and -CA option\n");
            goto end;
        }
    } else {
#define WARN_NO_CA(opt) BIO_printf(bio_err, \
        "Warning: ignoring " opt " option since -CA option is not given\n");
        if (CAkeyfile != NULL)
            WARN_NO_CA("-CAkey");
        if (CAkeyformat != FORMAT_UNDEF)
            WARN_NO_CA("-CAkeyform");
        if (CAformat != FORMAT_UNDEF)
            WARN_NO_CA("-CAform");
        if (CAserial != NULL)
            WARN_NO_CA("-CAserial");
        if (CA_createserial)
            WARN_NO_CA("-CAcreateserial");
    }

    if (extfile == NULL) {
        if (extsect != NULL)
        }
        if ((x = X509_new_ex(app_get0_libctx(), app_get0_propq())) == NULL)
            goto end;
        if (CAfile == NULL && sno == NULL) {
            sno = ASN1_INTEGER_new();
            if (sno == NULL || !rand_serial(NULL, sno))
                goto end;
        }
    char *buf = NULL;
    ASN1_INTEGER *bs = NULL;
    BIGNUM *serial = NULL;
    int defaultfile = 0, file_exists;

    if (serialfile == NULL) {
        const char *p = strrchr(CAfile, '.');
        size_t len = p != NULL ? (size_t)(p - CAfile) : strlen(CAfile);
        memcpy(buf, CAfile, len);
        memcpy(buf + len, POSTFIX, sizeof(POSTFIX));
        serialfile = buf;
        defaultfile = 1;
    }

    serial = load_serial(serialfile, &file_exists, create || defaultfile, NULL);
    if (serial == NULL)
        goto end;

    if (!BN_add_word(serial, 1)) {
        goto end;
    }

    if (file_exists || create)
        save_serial(serialfile, NULL, serial, &bs);
    else
        bs = BN_to_ASN1_INTEGER(serial, NULL);

 end:
    OPENSSL_free(buf);
    BN_free(serial);