                goto end;
            }
        } else {
            if ((serial = load_serial(serialfile, create_ser, NULL)) == NULL) {
                BIO_printf(bio_err, "error while loading serial number\n");
                goto end;
            }
            if (verbose) {

        if ((crlnumberfile = NCONF_get_string(conf, section, ENV_CRLNUMBER))
            != NULL)
            if ((crlnumber = load_serial(crlnumberfile, 0, NULL)) == NULL) {
                BIO_printf(bio_err, "error while loading CRL number\n");
                goto end;
            }
