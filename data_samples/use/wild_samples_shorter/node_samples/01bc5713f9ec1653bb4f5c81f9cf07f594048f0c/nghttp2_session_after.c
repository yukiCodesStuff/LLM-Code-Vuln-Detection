    return 0;
  }

  /* Ignore all incoming frames because we are going to tear down the
     session. */
  session->iframe.state = NGHTTP2_IB_IGN_ALL;

  if (reason == NULL) {
    debug_data = NULL;
    debug_datalen = 0;
  } else {
      assert(session->obq_flood_counter_ > 0);
      --session->obq_flood_counter_;
    }
    /* PING frame is allowed to be sent unless termination GOAWAY is
       sent */
    if (session->goaway_flags & NGHTTP2_GOAWAY_TERM_ON_SEND) {
      return NGHTTP2_ERR_SESSION_CLOSING;
    }
    nghttp2_frame_pack_ping(&session->aob.framebufs, &frame->ping);
    return 0;

      if (iframe->sbuf.pos[3] != NGHTTP2_SETTINGS ||
          (iframe->sbuf.pos[4] & NGHTTP2_FLAG_ACK)) {
        rv = session_call_error_callback(
            session, NGHTTP2_ERR_SETTINGS_EXPECTED,
            "Remote peer returned unexpected data while we expected "
            "SETTINGS frame.  Perhaps, peer does not support HTTP/2 "
        DEBUGF("recv: length is too large %zu > %u\n", iframe->frame.hd.length,
               session->local_settings.max_frame_size);

        rv = nghttp2_session_terminate_session_with_reason(
            session, NGHTTP2_FRAME_SIZE_ERROR, "too large frame size");

        if (nghttp2_is_fatal(rv)) {
          return rv;
        }

        return (ssize_t)inlen;
      }

      switch (iframe->frame.hd.type) {
      case NGHTTP2_DATA: {
        busy = 1;

        rv = session_on_data_received_fail_fast(session);
        if (iframe->state == NGHTTP2_IB_IGN_ALL) {
          return (ssize_t)inlen;
        }
        if (rv == NGHTTP2_ERR_IGN_PAYLOAD) {
          DEBUGF("recv: DATA not allowed stream_id=%d\n",
                 iframe->frame.hd.stream_id);
          iframe->state = NGHTTP2_IB_IGN_DATA;

        rv = inbound_frame_handle_pad(iframe, &iframe->frame.hd);
        if (rv < 0) {
          rv = nghttp2_session_terminate_session_with_reason(
              session, NGHTTP2_PROTOCOL_ERROR,
              "DATA: insufficient padding space");

          if (nghttp2_is_fatal(rv)) {
            return rv;
          }
          return (ssize_t)inlen;
        }

        if (rv == 1) {
          iframe->state = NGHTTP2_IB_READ_PAD_DATA;

        rv = inbound_frame_handle_pad(iframe, &iframe->frame.hd);
        if (rv < 0) {
          rv = nghttp2_session_terminate_session_with_reason(
              session, NGHTTP2_PROTOCOL_ERROR,
              "HEADERS: insufficient padding space");
          if (nghttp2_is_fatal(rv)) {
            return rv;
          }
          return (ssize_t)inlen;
        }

        if (rv == 1) {
          iframe->state = NGHTTP2_IB_READ_NBYTE;

        busy = 1;

        if (iframe->state == NGHTTP2_IB_IGN_ALL) {
          return (ssize_t)inlen;
        }

        if (rv == NGHTTP2_ERR_TEMPORAL_CALLBACK_FAILURE) {
          rv = nghttp2_session_add_rst_stream(
              session, iframe->frame.hd.stream_id, NGHTTP2_INTERNAL_ERROR);
          if (nghttp2_is_fatal(rv)) {

        rv = inbound_frame_handle_pad(iframe, &iframe->frame.hd);
        if (rv < 0) {
          rv = nghttp2_session_terminate_session_with_reason(
              session, NGHTTP2_PROTOCOL_ERROR,
              "PUSH_PROMISE: insufficient padding space");
          if (nghttp2_is_fatal(rv)) {
            return rv;
          }
          return (ssize_t)inlen;
        }

        if (rv == 1) {
          iframe->state = NGHTTP2_IB_READ_NBYTE;
          return rv;
        }

        return (ssize_t)inlen;
      default:
        DEBUGF("recv: extension frame\n");

        if (check_ext_type_set(session->user_recv_ext_types,
        case NGHTTP2_IB_IGN_PAYLOAD:
        case NGHTTP2_IB_FRAME_SIZE_ERROR:
        case NGHTTP2_IB_IGN_DATA:
        case NGHTTP2_IB_IGN_ALL:
          break;
        default:
          rv = session_call_on_begin_frame(session, &iframe->frame.hd);

      case NGHTTP2_HEADERS:
        if (iframe->padlen == 0 &&
            (iframe->frame.hd.flags & NGHTTP2_FLAG_PADDED)) {
          pri_fieldlen = nghttp2_frame_priority_len(iframe->frame.hd.flags);
          padlen = inbound_frame_compute_pad(iframe);
          if (padlen < 0 ||
              (size_t)padlen + pri_fieldlen > 1 + iframe->payloadleft) {
            rv = nghttp2_session_terminate_session_with_reason(
                session, NGHTTP2_PROTOCOL_ERROR, "HEADERS: invalid padding");
            if (nghttp2_is_fatal(rv)) {
              return rv;
            }
            return (ssize_t)inlen;
          }
          iframe->frame.headers.padlen = (size_t)padlen;

          if (pri_fieldlen > 0) {
            if (iframe->payloadleft < pri_fieldlen) {
              busy = 1;
              iframe->state = NGHTTP2_IB_FRAME_SIZE_ERROR;

        busy = 1;

        if (iframe->state == NGHTTP2_IB_IGN_ALL) {
          return (ssize_t)inlen;
        }

        if (rv == NGHTTP2_ERR_TEMPORAL_CALLBACK_FAILURE) {
          rv = nghttp2_session_add_rst_stream(
              session, iframe->frame.hd.stream_id, NGHTTP2_INTERNAL_ERROR);
          if (nghttp2_is_fatal(rv)) {
          return rv;
        }

        if (iframe->state == NGHTTP2_IB_IGN_ALL) {
          return (ssize_t)inlen;
        }

        session_inbound_frame_reset(session);

        break;
      case NGHTTP2_RST_STREAM:
          return rv;
        }

        if (iframe->state == NGHTTP2_IB_IGN_ALL) {
          return (ssize_t)inlen;
        }

        session_inbound_frame_reset(session);

        break;
      case NGHTTP2_PUSH_PROMISE:
        if (iframe->padlen == 0 &&
            (iframe->frame.hd.flags & NGHTTP2_FLAG_PADDED)) {
          padlen = inbound_frame_compute_pad(iframe);
          if (padlen < 0 || (size_t)padlen + 4 /* promised stream id */
                                > 1 + iframe->payloadleft) {
            rv = nghttp2_session_terminate_session_with_reason(
                session, NGHTTP2_PROTOCOL_ERROR,
                "PUSH_PROMISE: invalid padding");
            if (nghttp2_is_fatal(rv)) {
              return rv;
            }
            return (ssize_t)inlen;
          }

          iframe->frame.push_promise.padlen = (size_t)padlen;


        busy = 1;

        if (iframe->state == NGHTTP2_IB_IGN_ALL) {
          return (ssize_t)inlen;
        }

        if (rv == NGHTTP2_ERR_TEMPORAL_CALLBACK_FAILURE) {
          rv = nghttp2_session_add_rst_stream(
              session, iframe->frame.push_promise.promised_stream_id,
              NGHTTP2_INTERNAL_ERROR);
          return rv;
        }

        if (iframe->state == NGHTTP2_IB_IGN_ALL) {
          return (ssize_t)inlen;
        }

        session_inbound_frame_reset(session);

        break;
      case NGHTTP2_GOAWAY: {
          return rv;
        }

        if (iframe->state == NGHTTP2_IB_IGN_ALL) {
          return (ssize_t)inlen;
        }

        session_inbound_frame_reset(session);

        break;
      case NGHTTP2_ALTSVC: {

      data_readlen = inbound_frame_effective_readlen(
          iframe, iframe->payloadleft - readlen, readlen);

      if (data_readlen == -1) {
        /* everything is padding */
        data_readlen = 0;
      }

      trail_padlen = nghttp2_frame_trail_padlen(&iframe->frame, iframe->padlen);

      final = (iframe->frame.hd.flags & NGHTTP2_FLAG_END_HEADERS) &&
              iframe->payloadleft - (size_t)data_readlen == trail_padlen;
          return rv;
        }

        if (iframe->state == NGHTTP2_IB_IGN_ALL) {
          return (ssize_t)inlen;
        }

        if (rv == NGHTTP2_ERR_PAUSE) {
          in += hd_proclen;
          iframe->payloadleft -= hd_proclen;

        return rv;
      }

      assert(iframe->state == NGHTTP2_IB_IGN_ALL);

      return (ssize_t)inlen;
    case NGHTTP2_IB_READ_SETTINGS:
      DEBUGF("recv: [IB_READ_SETTINGS]\n");

      readlen = inbound_frame_buf_read(iframe, in, last);
        return rv;
      }

      if (iframe->state == NGHTTP2_IB_IGN_ALL) {
        return (ssize_t)inlen;
      }

      session_inbound_frame_reset(session);

      break;
    case NGHTTP2_IB_READ_GOAWAY_DEBUG:
        return rv;
      }

      if (iframe->state == NGHTTP2_IB_IGN_ALL) {
        return (ssize_t)inlen;
      }

      session_inbound_frame_reset(session);

      break;
    case NGHTTP2_IB_EXPECT_CONTINUATION:
          return rv;
        }

        return (ssize_t)inlen;
      }

      /* CONTINUATION won't bear NGHTTP2_PADDED flag */

        return rv;
      }

      if (iframe->state == NGHTTP2_IB_IGN_ALL) {
        return (ssize_t)inlen;
      }

      /* Pad Length field is consumed immediately */
      rv =
          nghttp2_session_consume(session, iframe->frame.hd.stream_id, readlen);

        return rv;
      }

      if (iframe->state == NGHTTP2_IB_IGN_ALL) {
        return (ssize_t)inlen;
      }

      stream = nghttp2_session_get_stream(session, iframe->frame.hd.stream_id);
      if (stream) {
        rv = session_update_recv_stream_window_size(
            session, stream, readlen,
        if (nghttp2_is_fatal(rv)) {
          return rv;
        }
        return (ssize_t)inlen;
      }

      iframe->frame.data.padlen = (size_t)padlen;

          return rv;
        }

        if (iframe->state == NGHTTP2_IB_IGN_ALL) {
          return (ssize_t)inlen;
        }

        rv = session_update_recv_stream_window_size(
            session, stream, readlen,
            iframe->payloadleft ||
                (iframe->frame.hd.flags & NGHTTP2_FLAG_END_STREAM) == 0);
          if (nghttp2_is_fatal(rv)) {
            return rv;
          }

          if (iframe->state == NGHTTP2_IB_IGN_ALL) {
            return (ssize_t)inlen;
          }
        }

        DEBUGF("recv: data_readlen=%zd\n", data_readlen);

                if (nghttp2_is_fatal(rv)) {
                  return rv;
                }

                if (iframe->state == NGHTTP2_IB_IGN_DATA) {
                  return (ssize_t)inlen;
                }
              }

              rv = nghttp2_session_add_rst_stream(
                  session, iframe->frame.hd.stream_id, NGHTTP2_PROTOCOL_ERROR);
          return rv;
        }

        if (iframe->state == NGHTTP2_IB_IGN_ALL) {
          return (ssize_t)inlen;
        }

        if (session->opt_flags & NGHTTP2_OPTMASK_NO_AUTO_WINDOW_UPDATE) {

          /* Ignored DATA is considered as "consumed" immediately. */
          rv = session_update_connection_consumed_size(session, readlen);
          if (nghttp2_is_fatal(rv)) {
            return rv;
          }

          if (iframe->state == NGHTTP2_IB_IGN_ALL) {
            return (ssize_t)inlen;
          }
        }
      }

      if (iframe->payloadleft) {
nghttp2_session_get_hd_deflate_dynamic_table_size(nghttp2_session *session) {
  return nghttp2_hd_deflate_get_dynamic_table_size(&session->hd_deflater);
}

void nghttp2_session_set_user_data(nghttp2_session *session, void *user_data) {
  session->user_data = user_data;
}