            aliasout = ++num;
            break;
        case OPT_CACREATESERIAL:
            CA_createserial = ++num;
            break;
        case OPT_CLREXT:
            clrext = 1;
            break;
            BIO_printf(bio_err, "Cannot use both -key/-signkey and -CA option\n");
            goto end;
        }
    } else if (CAkeyfile != NULL) {
        BIO_printf(bio_err,
                   "Warning: ignoring -CAkey option since no -CA option is given\n");
    }

    if (extfile == NULL) {
        if (extsect != NULL)
        }
        if ((x = X509_new_ex(app_get0_libctx(), app_get0_propq())) == NULL)
            goto end;
        if (sno == NULL) {
            sno = ASN1_INTEGER_new();
            if (sno == NULL || !rand_serial(NULL, sno))
                goto end;
        }
    char *buf = NULL;
    ASN1_INTEGER *bs = NULL;
    BIGNUM *serial = NULL;

    if (serialfile == NULL) {
        const char *p = strrchr(CAfile, '.');
        size_t len = p != NULL ? (size_t)(p - CAfile) : strlen(CAfile);
        memcpy(buf, CAfile, len);
        memcpy(buf + len, POSTFIX, sizeof(POSTFIX));
        serialfile = buf;
    }

    serial = load_serial(serialfile, create, NULL);
    if (serial == NULL)
        goto end;

    if (!BN_add_word(serial, 1)) {
        goto end;
    }

    if (!save_serial(serialfile, NULL, serial, &bs))
        goto end;

 end:
    OPENSSL_free(buf);
    BN_free(serial);