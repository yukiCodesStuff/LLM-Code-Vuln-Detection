     * (2) update s->init_num
     */
    pitem *item;
    piterator iter;
    hm_fragment *frag;
    int ret;
    int chretran = 0;

    iter = pqueue_iterator(s->d1->buffered_messages);
    do {
        item = pqueue_next(&iter);
        if (item == NULL)
            return 0;

        frag = (hm_fragment *)item->data;

        if (frag->msg_header.seq < s->d1->handshake_read_seq) {
            pitem *next;
            hm_fragment *nextfrag;

            if (!s->server
                    || frag->msg_header.seq != 0
                    || s->d1->handshake_read_seq != 1
                    || s->statem.hand_state != DTLS_ST_SW_HELLO_VERIFY_REQUEST) {
                /*
                 * This is a stale message that has been buffered so clear it.
                 * It is safe to pop this message from the queue even though
                 * we have an active iterator
                 */
                pqueue_pop(s->d1->buffered_messages);
                dtls1_hm_fragment_free(frag);
                pitem_free(item);
                item = NULL;
                frag = NULL;
            } else {
                /*
                 * We have fragments for a ClientHello without a cookie,
                 * even though we have sent a HelloVerifyRequest. It is possible
                 * that the HelloVerifyRequest got lost and this is a
                 * retransmission of the original ClientHello
                 */
                next = pqueue_next(&iter);
                if (next != NULL) {
                    nextfrag = (hm_fragment *)next->data;
                    if (nextfrag->msg_header.seq == s->d1->handshake_read_seq) {
                        /*
                        * We have fragments for both a ClientHello without
                        * cookie and one with. Ditch the one without.
                        */
                        pqueue_pop(s->d1->buffered_messages);
                        dtls1_hm_fragment_free(frag);
                        pitem_free(item);
                        item = next;
                        frag = nextfrag;
                    } else {
                        chretran = 1;
                    }
                } else {
                    chretran = 1;
                }
            }
        }
    } while (item == NULL);

    /* Don't return if reassembly still in progress */
    if (frag->reassembly != NULL)
        return 0;

    if (s->d1->handshake_read_seq == frag->msg_header.seq || chretran) {
        size_t frag_len = frag->msg_header.frag_len;
        pqueue_pop(s->d1->buffered_messages);

        /* Calls SSLfatal() as required */
        pitem_free(item);

        if (ret) {
            if (chretran) {
                /*
                 * We got a new ClientHello with a message sequence of 0.
                 * Reset the read/write sequences back to the beginning.
                 * We process it like this is the first time we've seen a
                 * ClientHello from the client.
                 */
                s->d1->handshake_read_seq = 0;
                s->d1->next_handshake_write_seq = 0;
            }
            *len = frag_len;
            return 1;
        }

    int i, ret, recvd_type;
    struct hm_header_st msg_hdr;
    size_t readbytes;
    int chretran = 0;

    *errtype = 0;

 redo:
     * although we're still expecting seq 0 (ClientHello)
     */
    if (msg_hdr.seq != s->d1->handshake_read_seq) {
        if (!s->server
                || msg_hdr.seq != 0
                || s->d1->handshake_read_seq != 1
                || wire[0] != SSL3_MT_CLIENT_HELLO
                || s->statem.hand_state != DTLS_ST_SW_HELLO_VERIFY_REQUEST) {
            *errtype = dtls1_process_out_of_seq_message(s, &msg_hdr);
            return 0;
        }
        /*
         * We received a ClientHello and sent back a HelloVerifyRequest. We
         * now seem to have received a retransmitted initial ClientHello. That
         * is allowed (possibly our HelloVerifyRequest got lost).
         */
        chretran = 1;
    }

    if (frag_len && frag_len < mlen) {
        *errtype = dtls1_reassemble_fragment(s, &msg_hdr);
        goto f_err;
    }

    if (chretran) {
        /*
         * We got a new ClientHello with a message sequence of 0.
         * Reset the read/write sequences back to the beginning.
         * We process it like this is the first time we've seen a ClientHello
         * from the client.
         */
        s->d1->handshake_read_seq = 0;
        s->d1->next_handshake_write_seq = 0;
    }

    /*
     * Note that s->init_num is *not* used as current offset in
     * s->init_buf->data, but as a counter summing up fragments' lengths: as
     * soon as they sum up to handshake packet length, we assume we have got