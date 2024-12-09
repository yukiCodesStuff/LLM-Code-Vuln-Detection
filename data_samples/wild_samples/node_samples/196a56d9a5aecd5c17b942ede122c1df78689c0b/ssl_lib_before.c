{
    /*
     * Similar to SSL_pending() but returns a 1 to indicate that we have
     * unprocessed data available or 0 otherwise (as opposed to the number of
     * bytes available). Unlike SSL_pending() this will take into account
     * read_ahead data. A 1 return simply indicates that we have unprocessed
     * data. That data may not result in any application data, or we may fail
     * to parse the records for some reason.
     */
    if (RECORD_LAYER_processed_read_pending(&s->rlayer))
        return 1;

    return RECORD_LAYER_read_pending(&s->rlayer);
}

X509 *SSL_get1_peer_certificate(const SSL *s)
{
    X509 *r = SSL_get0_peer_certificate(s);

    if (r != NULL)
        X509_up_ref(r);

    return r;
}

X509 *SSL_get0_peer_certificate(const SSL *s)
{
    if ((s == NULL) || (s->session == NULL))
        return NULL;
    else
        return s->session->peer;
}

STACK_OF(X509) *SSL_get_peer_cert_chain(const SSL *s)
{
    STACK_OF(X509) *r;

    if ((s == NULL) || (s->session == NULL))
        r = NULL;
    else
        r = s->session->peer_chain;

    /*
     * If we are a client, cert_chain includes the peer's own certificate; if
     * we are a server, it does not.
     */

    return r;
}

/*
 * Now in theory, since the calling process own 't' it should be safe to
 * modify.  We need to be able to read f without being hassled
 */
int SSL_copy_session_id(SSL *t, const SSL *f)
{
    int i;
    /* Do we need to do SSL locking? */
    if (!SSL_set_session(t, SSL_get_session(f))) {
        return 0;
    }