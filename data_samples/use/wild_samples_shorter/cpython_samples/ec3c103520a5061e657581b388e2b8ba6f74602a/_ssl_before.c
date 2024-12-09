
    int i, j;
    PyObject *peer_alt_names = Py_None;
    PyObject *v, *t;
    X509_EXTENSION *ext = NULL;
    GENERAL_NAMES *names = NULL;
    GENERAL_NAME *name;
    const X509V3_EXT_METHOD *method;
                           ext->value->length));

        for(j = 0; j < sk_GENERAL_NAME_num(names); j++) {

            /* get a rendering of each name in the set of names */

            name = sk_GENERAL_NAME_value(names, j);
            if (name->type == GEN_DIRNAME) {

                /* we special-case DirName as a tuple of
                   tuples of attributes */

                t = PyTuple_New(2);
                    goto fail;
                }
                PyTuple_SET_ITEM(t, 1, v);

            } else {

                /* for everything else, we use the OpenSSL print form */

                (void) BIO_reset(biobuf);
                GENERAL_NAME_print(biobuf, name);
                len = BIO_gets(biobuf, buf, sizeof(buf)-1);
                if (len < 0) {
                    goto fail;
                }
                PyTuple_SET_ITEM(t, 1, v);
            }

            /* and add that rendering to the list */
