        if(!(method = X509V3_EXT_get(ext))) {
            PyErr_SetString
              (PySSLErrorObject,
               ERRSTR("No method for internalizing subjectAltName!"));
            goto fail;
        }

        p = ext->value->data;
        if (method->it)
            names = (GENERAL_NAMES*)
              (ASN1_item_d2i(NULL,
                             &p,
                             ext->value->length,
                             ASN1_ITEM_ptr(method->it)));
        else
            names = (GENERAL_NAMES*)
              (method->d2i(NULL,
                           &p,
                           ext->value->length));

        for(j = 0; j < sk_GENERAL_NAME_num(names); j++) {

            /* get a rendering of each name in the set of names */

            name = sk_GENERAL_NAME_value(names, j);
            if (name->type == GEN_DIRNAME) {

                /* we special-case DirName as a tuple of
                   tuples of attributes */

                t = PyTuple_New(2);
                if (t == NULL) {
                    goto fail;
                }

                v = PyUnicode_FromString("DirName");
                if (v == NULL) {
                    Py_DECREF(t);
                    goto fail;
                }
                PyTuple_SET_ITEM(t, 0, v);

                v = _create_tuple_for_X509_NAME (name->d.dirn);
                if (v == NULL) {
                    Py_DECREF(t);
                    goto fail;
                }
                PyTuple_SET_ITEM(t, 1, v);

            } else {

                /* for everything else, we use the OpenSSL print form */

                (void) BIO_reset(biobuf);
                GENERAL_NAME_print(biobuf, name);
                len = BIO_gets(biobuf, buf, sizeof(buf)-1);
                if (len < 0) {
                    _setSSLError(NULL, 0, __FILE__, __LINE__);
                    goto fail;
                }
                vptr = strchr(buf, ':');
                if (vptr == NULL)
                    goto fail;
                t = PyTuple_New(2);
                if (t == NULL)
                    goto fail;
                v = PyUnicode_FromStringAndSize(buf, (vptr - buf));
                if (v == NULL) {
                    Py_DECREF(t);
                    goto fail;
                }
                PyTuple_SET_ITEM(t, 0, v);
                v = PyUnicode_FromStringAndSize((vptr + 1),
                                                (len - (vptr - buf + 1)));
                if (v == NULL) {
                    Py_DECREF(t);
                    goto fail;
                }
                PyTuple_SET_ITEM(t, 1, v);
            }

            /* and add that rendering to the list */

            if (PyList_Append(peer_alt_names, t) < 0) {
                Py_DECREF(t);
                goto fail;
            }
            Py_DECREF(t);
        }
        sk_GENERAL_NAME_pop_free(names, GENERAL_NAME_free);
    }
                if (v == NULL) {
                    Py_DECREF(t);
                    goto fail;
                }
                PyTuple_SET_ITEM(t, 1, v);

            } else {

                /* for everything else, we use the OpenSSL print form */

                (void) BIO_reset(biobuf);
                GENERAL_NAME_print(biobuf, name);
                len = BIO_gets(biobuf, buf, sizeof(buf)-1);
                if (len < 0) {
                    _setSSLError(NULL, 0, __FILE__, __LINE__);
                    goto fail;
                }
                if (v == NULL) {
                    Py_DECREF(t);
                    goto fail;
                }
                PyTuple_SET_ITEM(t, 1, v);
            }

            /* and add that rendering to the list */

            if (PyList_Append(peer_alt_names, t) < 0) {
                Py_DECREF(t);
                goto fail;
            }
            Py_DECREF(t);
        }
        sk_GENERAL_NAME_pop_free(names, GENERAL_NAME_free);
    }
    BIO_free(biobuf);
    if (peer_alt_names != Py_None) {
        v = PyList_AsTuple(peer_alt_names);
        Py_DECREF(peer_alt_names);
        return v;
    } else {
        return peer_alt_names;
    }


  fail:
    if (biobuf != NULL)
        BIO_free(biobuf);

    if (peer_alt_names != Py_None) {
        Py_XDECREF(peer_alt_names);
    }

    return NULL;
}

static PyObject *
_decode_certificate(X509 *certificate) {

    PyObject *retval = NULL;
    BIO *biobuf = NULL;
    PyObject *peer;
    PyObject *peer_alt_names = NULL;
    PyObject *issuer;
    PyObject *version;
    PyObject *sn_obj;
    ASN1_INTEGER *serialNumber;
    char buf[2048];
    int len;
    ASN1_TIME *notBefore, *notAfter;
    PyObject *pnotBefore, *pnotAfter;

    retval = PyDict_New();
    if (retval == NULL)
        return NULL;

    peer = _create_tuple_for_X509_NAME(
        X509_get_subject_name(certificate));
    if (peer == NULL)
        goto fail0;
    if (PyDict_SetItemString(retval, (const char *) "subject", peer) < 0) {
        Py_DECREF(peer);
        goto fail0;
    }
    Py_DECREF(peer);

    issuer = _create_tuple_for_X509_NAME(
        X509_get_issuer_name(certificate));
    if (issuer == NULL)
        goto fail0;
    if (PyDict_SetItemString(retval, (const char *)"issuer", issuer) < 0) {
        Py_DECREF(issuer);
        goto fail0;
    }
    Py_DECREF(issuer);

    version = PyLong_FromLong(X509_get_version(certificate) + 1);
    if (version == NULL)
        goto fail0;
    if (PyDict_SetItemString(retval, "version", version) < 0) {
        Py_DECREF(version);
        goto fail0;
    }
    Py_DECREF(version);

    /* get a memory buffer */
    biobuf = BIO_new(BIO_s_mem());

    (void) BIO_reset(biobuf);
    serialNumber = X509_get_serialNumber(certificate);
    /* should not exceed 20 octets, 160 bits, so buf is big enough */
    i2a_ASN1_INTEGER(biobuf, serialNumber);
    len = BIO_gets(biobuf, buf, sizeof(buf)-1);
    if (len < 0) {
        _setSSLError(NULL, 0, __FILE__, __LINE__);
        goto fail1;
    }
    sn_obj = PyUnicode_FromStringAndSize(buf, len);
    if (sn_obj == NULL)
        goto fail1;
    if (PyDict_SetItemString(retval, "serialNumber", sn_obj) < 0) {
        Py_DECREF(sn_obj);
        goto fail1;
    }
    Py_DECREF(sn_obj);

    (void) BIO_reset(biobuf);
    notBefore = X509_get_notBefore(certificate);
    ASN1_TIME_print(biobuf, notBefore);
    len = BIO_gets(biobuf, buf, sizeof(buf)-1);
    if (len < 0) {
        _setSSLError(NULL, 0, __FILE__, __LINE__);
        goto fail1;
    }
    pnotBefore = PyUnicode_FromStringAndSize(buf, len);
    if (pnotBefore == NULL)
        goto fail1;
    if (PyDict_SetItemString(retval, "notBefore", pnotBefore) < 0) {
        Py_DECREF(pnotBefore);
        goto fail1;
    }
    Py_DECREF(pnotBefore);

    (void) BIO_reset(biobuf);
    notAfter = X509_get_notAfter(certificate);
    ASN1_TIME_print(biobuf, notAfter);
    len = BIO_gets(biobuf, buf, sizeof(buf)-1);
    if (len < 0) {
        _setSSLError(NULL, 0, __FILE__, __LINE__);
        goto fail1;
    }
    pnotAfter = PyUnicode_FromStringAndSize(buf, len);
    if (pnotAfter == NULL)
        goto fail1;
    if (PyDict_SetItemString(retval, "notAfter", pnotAfter) < 0) {
        Py_DECREF(pnotAfter);
        goto fail1;
    }
    Py_DECREF(pnotAfter);

    /* Now look for subjectAltName */

    peer_alt_names = _get_peer_alt_names(certificate);
    if (peer_alt_names == NULL)
        goto fail1;
    else if (peer_alt_names != Py_None) {
        if (PyDict_SetItemString(retval, "subjectAltName",
                                 peer_alt_names) < 0) {
            Py_DECREF(peer_alt_names);
            goto fail1;
        }
        Py_DECREF(peer_alt_names);
    }

    BIO_free(biobuf);
    return retval;

  fail1:
    if (biobuf != NULL)
        BIO_free(biobuf);
  fail0:
    Py_XDECREF(retval);
    return NULL;
}


static PyObject *
PySSL_test_decode_certificate (PyObject *mod, PyObject *args) {

    PyObject *retval = NULL;
    PyObject *filename;
    X509 *x=NULL;
    BIO *cert;

    if (!PyArg_ParseTuple(args, "O&:test_decode_certificate",
                          PyUnicode_FSConverter, &filename))
        return NULL;

    if ((cert=BIO_new(BIO_s_file())) == NULL) {
        PyErr_SetString(PySSLErrorObject,
                        "Can't malloc memory to read file");
        goto fail0;
    }

    if (BIO_read_filename(cert, PyBytes_AsString(filename)) <= 0) {
        PyErr_SetString(PySSLErrorObject,
                        "Can't open file");
        goto fail0;
    }

    x = PEM_read_bio_X509_AUX(cert,NULL, NULL, NULL);
    if (x == NULL) {
        PyErr_SetString(PySSLErrorObject,
                        "Error decoding PEM-encoded file");
        goto fail0;
    }

    retval = _decode_certificate(x);
    X509_free(x);

  fail0:
    Py_DECREF(filename);
    if (cert != NULL) BIO_free(cert);
    return retval;
}


static PyObject *
PySSL_peercert(PySSLSocket *self, PyObject *args)
{
    PyObject *retval = NULL;
    int len;
    int verification;
    int binary_mode = 0;

    if (!PyArg_ParseTuple(args, "|p:peer_certificate", &binary_mode))
        return NULL;

    if (!self->peer_cert)
        Py_RETURN_NONE;

    if (binary_mode) {
        /* return cert in DER-encoded format */

        unsigned char *bytes_buf = NULL;

        bytes_buf = NULL;
        len = i2d_X509(self->peer_cert, &bytes_buf);
        if (len < 0) {
            PySSL_SetError(self, len, __FILE__, __LINE__);
            return NULL;
        }
        /* this is actually an immutable bytes sequence */
        retval = PyBytes_FromStringAndSize
          ((const char *) bytes_buf, len);
        OPENSSL_free(bytes_buf);
        return retval;

    } else {
        verification = SSL_CTX_get_verify_mode(SSL_get_SSL_CTX(self->ssl));
        if ((verification & SSL_VERIFY_PEER) == 0)
            return PyDict_New();
        else
            return _decode_certificate(self->peer_cert);
    }
}

PyDoc_STRVAR(PySSL_peercert_doc,
"peer_certificate([der=False]) -> certificate\n\
\n\
Returns the certificate for the peer.  If no certificate was provided,\n\
returns None.  If a certificate was provided, but not validated, returns\n\
an empty dictionary.  Otherwise returns a dict containing information\n\
about the peer certificate.\n\
\n\
If the optional argument is True, returns a DER-encoded copy of the\n\
peer certificate, or None if no certificate was provided.  This will\n\
return the certificate even if it wasn't validated.");

static PyObject *PySSL_cipher (PySSLSocket *self) {

    PyObject *retval, *v;
    const SSL_CIPHER *current;
    char *cipher_name;
    char *cipher_protocol;

    if (self->ssl == NULL)
        Py_RETURN_NONE;
    current = SSL_get_current_cipher(self->ssl);
    if (current == NULL)
        Py_RETURN_NONE;

    retval = PyTuple_New(3);
    if (retval == NULL)
        return NULL;

    cipher_name = (char *) SSL_CIPHER_get_name(current);
    if (cipher_name == NULL) {
        Py_INCREF(Py_None);
        PyTuple_SET_ITEM(retval, 0, Py_None);
    } else {
        v = PyUnicode_FromString(cipher_name);
        if (v == NULL)
            goto fail0;
        PyTuple_SET_ITEM(retval, 0, v);
    }
    cipher_protocol = SSL_CIPHER_get_version(current);
    if (cipher_protocol == NULL) {
        Py_INCREF(Py_None);
        PyTuple_SET_ITEM(retval, 1, Py_None);
    } else {
        v = PyUnicode_FromString(cipher_protocol);
        if (v == NULL)
            goto fail0;
        PyTuple_SET_ITEM(retval, 1, v);
    }