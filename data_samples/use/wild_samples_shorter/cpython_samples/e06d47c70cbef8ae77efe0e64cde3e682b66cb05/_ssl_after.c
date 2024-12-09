                           ext->value->length));

        for(j = 0; j < sk_GENERAL_NAME_num(names); j++) {
            /* get a rendering of each name in the set of names */
            int gntype;
            ASN1_STRING *as = NULL;

            name = sk_GENERAL_NAME_value(names, j);
            gntype = name-> type;
            switch (gntype) {
            case GEN_DIRNAME:
                /* we special-case DirName as a tuple of
                   tuples of attributes */

                t = PyTuple_New(2);
                    goto fail;
                }
                PyTuple_SET_ITEM(t, 1, v);
                break;

            case GEN_EMAIL:
            case GEN_DNS:
            case GEN_URI:
                /* GENERAL_NAME_print() doesn't handle NULL bytes in ASN1_string
                   correctly, CVE-2013-4238 */
                t = PyTuple_New(2);
                if (t == NULL)
                    goto fail;
                switch (gntype) {
                case GEN_EMAIL:
                    v = PyUnicode_FromString("email");
                    as = name->d.rfc822Name;
                    break;
                case GEN_DNS:
                    v = PyUnicode_FromString("DNS");
                    as = name->d.dNSName;
                    break;
                case GEN_URI:
                    v = PyUnicode_FromString("URI");
                    as = name->d.uniformResourceIdentifier;
                    break;
                }
                if (v == NULL) {
                    Py_DECREF(t);
                    goto fail;
                }
                PyTuple_SET_ITEM(t, 0, v);
                v = PyUnicode_FromStringAndSize((char *)ASN1_STRING_data(as),
                                                ASN1_STRING_length(as));
                if (v == NULL) {
                    Py_DECREF(t);
                    goto fail;
                }
                PyTuple_SET_ITEM(t, 1, v);
                break;

            default:
                /* for everything else, we use the OpenSSL print form */
                switch (gntype) {
                    /* check for new general name type */
                    case GEN_OTHERNAME:
                    case GEN_X400:
                    case GEN_EDIPARTY:
                    case GEN_IPADD:
                    case GEN_RID:
                        break;
                    default:
                        if (PyErr_WarnFormat(PyExc_RuntimeWarning, 1,
                                             "Unknown general name type %d",
                                             gntype) == -1) {
                            goto fail;
                        }
                        break;
                }
                (void) BIO_reset(biobuf);
                GENERAL_NAME_print(biobuf, name);
                len = BIO_gets(biobuf, buf, sizeof(buf)-1);
                if (len < 0) {
                    goto fail;
                }
                PyTuple_SET_ITEM(t, 1, v);
                break;
            }

            /* and add that rendering to the list */
