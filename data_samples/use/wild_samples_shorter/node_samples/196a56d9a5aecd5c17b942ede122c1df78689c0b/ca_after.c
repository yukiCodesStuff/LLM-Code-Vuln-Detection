                goto end;
            }
        } else {
            serial = load_serial(serialfile, NULL, create_ser, NULL);
            if (serial == NULL) {
                BIO_printf(bio_err, "error while loading serial number\n");
                goto end;
            }
            if (verbose) {

        if ((crlnumberfile = NCONF_get_string(conf, section, ENV_CRLNUMBER))
            != NULL)
            if ((crlnumber = load_serial(crlnumberfile, NULL, 0, NULL))
                == NULL) {
                BIO_printf(bio_err, "error while loading CRL number\n");
                goto end;
            }
