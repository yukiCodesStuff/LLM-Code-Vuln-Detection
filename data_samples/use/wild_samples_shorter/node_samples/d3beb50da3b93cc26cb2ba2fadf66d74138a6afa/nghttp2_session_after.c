
  (*session_ptr)->max_send_header_block_length = NGHTTP2_MAX_HEADERSLEN;
  (*session_ptr)->max_outbound_ack = NGHTTP2_DEFAULT_MAX_OBQ_FLOOD_ITEM;
  (*session_ptr)->max_settings = NGHTTP2_DEFAULT_MAX_SETTINGS;

  if (option) {
    if ((option->opt_set_mask & NGHTTP2_OPT_NO_AUTO_WINDOW_UPDATE) &&
        option->no_auto_window_update) {
    if (option->opt_set_mask & NGHTTP2_OPT_MAX_OUTBOUND_ACK) {
      (*session_ptr)->max_outbound_ack = option->max_outbound_ack;
    }

    if ((option->opt_set_mask & NGHTTP2_OPT_MAX_SETTINGS) &&
        option->max_settings) {
      (*session_ptr)->max_settings = option->max_settings;
    }
  }

  rv = nghttp2_hd_deflate_init2(&(*session_ptr)->hd_deflater,
                                max_deflate_dynamic_table_size, mem);
static int session_update_connection_consumed_size(nghttp2_session *session,
                                                   size_t delta_size);

/*
 * Called after a frame is sent.  This function runs
 * on_frame_send_callback and handles stream closure upon END_STREAM
 * or RST_STREAM.  This function does not reset session->aob.  It is a
      if (session->opt_flags & NGHTTP2_OPTMASK_NO_AUTO_WINDOW_UPDATE) {
        rv = session_update_connection_consumed_size(session, 0);
      } else {
        rv = nghttp2_session_update_recv_connection_window_size(session, 0);
      }

      if (nghttp2_is_fatal(rv)) {
        return rv;
    if (session->opt_flags & NGHTTP2_OPTMASK_NO_AUTO_WINDOW_UPDATE) {
      rv = session_update_stream_consumed_size(session, stream, 0);
    } else {
      rv =
          nghttp2_session_update_recv_stream_window_size(session, stream, 0, 1);
    }

    if (nghttp2_is_fatal(rv)) {
      return rv;
  return 0;
}

int nghttp2_session_update_recv_stream_window_size(nghttp2_session *session,
                                                   nghttp2_stream *stream,
                                                   size_t delta_size,
                                                   int send_window_update) {
  int rv;
  rv = adjust_recv_window_size(&stream->recv_window_size, delta_size,
                               stream->local_window_size);
  if (rv != 0) {
  return 0;
}

int nghttp2_session_update_recv_connection_window_size(nghttp2_session *session,
                                                       size_t delta_size) {
  int rv;
  rv = adjust_recv_window_size(&session->recv_window_size, delta_size,
                               session->local_window_size);
  if (rv != 0) {
          break;
        }

        /* Check the settings flood counter early to be safe */
        if (session->obq_flood_counter_ >= session->max_outbound_ack &&
            !(iframe->frame.hd.flags & NGHTTP2_FLAG_ACK)) {
          return NGHTTP2_ERR_FLOODED;
        }

        iframe->state = NGHTTP2_IB_READ_SETTINGS;

        if (iframe->payloadleft) {
          nghttp2_settings_entry *min_header_table_size_entry;
          iframe->max_niv =
              iframe->frame.hd.length / NGHTTP2_FRAME_SETTINGS_ENTRY_LENGTH + 1;

          if (iframe->max_niv - 1 > session->max_settings) {
            rv = nghttp2_session_terminate_session_with_reason(
                session, NGHTTP2_ENHANCE_YOUR_CALM,
                "SETTINGS: too many setting entries");
            if (nghttp2_is_fatal(rv)) {
              return rv;
            }
            return (ssize_t)inlen;
          }

          iframe->iv = nghttp2_mem_malloc(mem, sizeof(nghttp2_settings_entry) *
                                                   iframe->max_niv);

          if (!iframe->iv) {
      }

      /* Pad Length field is subject to flow control */
      rv = nghttp2_session_update_recv_connection_window_size(session, readlen);
      if (nghttp2_is_fatal(rv)) {
        return rv;
      }


      stream = nghttp2_session_get_stream(session, iframe->frame.hd.stream_id);
      if (stream) {
        rv = nghttp2_session_update_recv_stream_window_size(
            session, stream, readlen,
            iframe->payloadleft ||
                (iframe->frame.hd.flags & NGHTTP2_FLAG_END_STREAM) == 0);
        if (nghttp2_is_fatal(rv)) {
      if (readlen > 0) {
        ssize_t data_readlen;

        rv = nghttp2_session_update_recv_connection_window_size(session,
                                                                readlen);
        if (nghttp2_is_fatal(rv)) {
          return rv;
        }

          return (ssize_t)inlen;
        }

        rv = nghttp2_session_update_recv_stream_window_size(
            session, stream, readlen,
            iframe->payloadleft ||
                (iframe->frame.hd.flags & NGHTTP2_FLAG_END_STREAM) == 0);
        if (nghttp2_is_fatal(rv)) {
      if (readlen > 0) {
        /* Update connection-level flow control window for ignored
           DATA frame too */
        rv = nghttp2_session_update_recv_connection_window_size(session,
                                                                readlen);
        if (nghttp2_is_fatal(rv)) {
          return rv;
        }

  if (settings_payloadlen % NGHTTP2_FRAME_SETTINGS_ENTRY_LENGTH) {
    return NGHTTP2_ERR_INVALID_ARGUMENT;
  }
  /* SETTINGS frame contains too many settings */
  if (settings_payloadlen / NGHTTP2_FRAME_SETTINGS_ENTRY_LENGTH
        > session->max_settings) {
    return NGHTTP2_ERR_TOO_MANY_SETTINGS;
  }
  rv = nghttp2_frame_unpack_settings_payload2(&iv, &niv, settings_payload,
                                              settings_payloadlen, mem);
  if (rv != 0) {
    return rv;