     * (2) update s->init_num
     */
    pitem *item;
    hm_fragment *frag;
    int ret;

    do {
        item = pqueue_peek(s->d1->buffered_messages);
        if (item == NULL)
            return 0;

        frag = (hm_fragment *)item->data;

        if (frag->msg_header.seq < s->d1->handshake_read_seq) {
            /* This is a stale message that has been buffered so clear it */
            pqueue_pop(s->d1->buffered_messages);
            dtls1_hm_fragment_free(frag);
            pitem_free(item);
            item = NULL;
            frag = NULL;
        }
    } while (item == NULL);

    /* Don't return if reassembly still in progress */
    if (frag->reassembly != NULL)
        return 0;

    if (s->d1->handshake_read_seq == frag->msg_header.seq) {
        size_t frag_len = frag->msg_header.frag_len;
        pqueue_pop(s->d1->buffered_messages);

        /* Calls SSLfatal() as required */
        pitem_free(item);

        if (ret) {
            *len = frag_len;
            return 1;
        }

    int i, ret, recvd_type;
    struct hm_header_st msg_hdr;
    size_t readbytes;

    *errtype = 0;

 redo:
     * although we're still expecting seq 0 (ClientHello)
     */
    if (msg_hdr.seq != s->d1->handshake_read_seq) {
        *errtype = dtls1_process_out_of_seq_message(s, &msg_hdr);
        return 0;
    }

    if (frag_len && frag_len < mlen) {
        *errtype = dtls1_reassemble_fragment(s, &msg_hdr);
        goto f_err;
    }

    /*
     * Note that s->init_num is *not* used as current offset in
     * s->init_buf->data, but as a counter summing up fragments' lengths: as
     * soon as they sum up to handshake packet length, we assume we have got