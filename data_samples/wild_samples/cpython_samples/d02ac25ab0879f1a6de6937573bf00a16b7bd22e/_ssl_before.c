    if (self->ctx->check_hostname) {
        X509_VERIFY_PARAM *param = SSL_get0_param(self->ssl);
        if (ip == NULL) {
            if (!X509_VERIFY_PARAM_set1_host(param, server_hostname, 0)) {
                _setSSLError(NULL, 0, __FILE__, __LINE__);
                goto error;
            }
        } else {
            if (!X509_VERIFY_PARAM_set1_ip(param, ASN1_STRING_data(ip),
                                           ASN1_STRING_length(ip))) {
                _setSSLError(NULL, 0, __FILE__, __LINE__);
                goto error;
            }
        }
    }
{
    char *hostname = NULL;
    PyObject *res;

    /* server_hostname is either None (or absent), or to be encoded
       as IDN A-label (ASCII str). */
    if (hostname_obj != Py_None) {
        if (!PyArg_Parse(hostname_obj, "es", "ascii", &hostname))
            return NULL;
    }
{
    char *hostname = NULL;
    PyObject *res;

    /* server_hostname is either None (or absent), or to be encoded
       as IDN A-label (ASCII str). */
    if (hostname_obj != Py_None) {
        if (!PyArg_Parse(hostname_obj, "es", "ascii", &hostname))
            return NULL;
    }