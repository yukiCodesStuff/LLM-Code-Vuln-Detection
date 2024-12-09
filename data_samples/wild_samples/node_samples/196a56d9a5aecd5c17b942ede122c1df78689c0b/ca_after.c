            if ((serial = BN_new()) == NULL || !rand_serial(serial, NULL)) {
                BIO_printf(bio_err, "error generating serial number\n");
                goto end;
            }
        } else {
            serial = load_serial(serialfile, NULL, create_ser, NULL);
            if (serial == NULL) {
                BIO_printf(bio_err, "error while loading serial number\n");
                goto end;
            }
            if (!X509V3_EXT_add_nconf(conf, &ctx, crl_ext, NULL)) {
                BIO_printf(bio_err,
                           "Error checking CRL extension section %s\n", crl_ext);
                ret = 1;
                goto end;
            }
        }

        if ((crlnumberfile = NCONF_get_string(conf, section, ENV_CRLNUMBER))
            != NULL)
            if ((crlnumber = load_serial(crlnumberfile, NULL, 0, NULL))
                == NULL) {
                BIO_printf(bio_err, "error while loading CRL number\n");
                goto end;
            }