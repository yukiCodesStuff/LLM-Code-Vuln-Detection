{
    /*
     * Similar to SSL_pending() but returns a 1 to indicate that we have
     * processed or unprocessed data available or 0 otherwise (as opposed to the
     * number of bytes available). Unlike SSL_pending() this will take into
     * account read_ahead data. A 1 return simply indicates that we have data.
     * That data may not result in any application data, or we may fail to parse
     * the records for some reason.
     */

    /* Check buffered app data if any first */
    if (SSL_IS_DTLS(s)) {
        DTLS1_RECORD_DATA *rdata;
        pitem *item, *iter;

        iter = pqueue_iterator(s->rlayer.d->buffered_app_data.q);
        while ((item = pqueue_next(&iter)) != NULL) {
            rdata = item->data;
            if (rdata->rrec.length > 0)
                return 1;
        }
    }

    if (RECORD_LAYER_processed_read_pending(&s->rlayer))
        return 1;

    return RECORD_LAYER_read_pending(&s->rlayer);