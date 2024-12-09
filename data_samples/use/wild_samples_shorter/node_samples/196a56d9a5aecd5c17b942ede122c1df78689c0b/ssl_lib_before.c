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