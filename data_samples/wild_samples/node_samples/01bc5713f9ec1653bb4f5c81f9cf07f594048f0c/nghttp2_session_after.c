  if (session->goaway_flags & NGHTTP2_GOAWAY_TERM_ON_SEND) {
    return 0;
  }

  /* Ignore all incoming frames because we are going to tear down the
     session. */
  session->iframe.state = NGHTTP2_IB_IGN_ALL;

  if (reason == NULL) {
    debug_data = NULL;
    debug_datalen = 0;
  } else {
    debug_data = (const uint8_t *)reason;
    debug_datalen = strlen(reason);
  }
    if (frame->hd.flags & NGHTTP2_FLAG_ACK) {
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
  case NGHTTP2_GOAWAY:
    rv = nghttp2_frame_pack_goaway(&session->aob.framebufs, &frame->goaway);
    if (rv != 0) {
      return rv;
    }
    session->local_last_stream_id = frame->goaway.last_stream_id;

    return 0;
  case NGHTTP2_WINDOW_UPDATE:
    rv = session_predicate_window_update_send(session, frame->hd.stream_id);
    if (rv != 0) {
      return rv;
    }
    nghttp2_frame_pack_window_update(&session->aob.framebufs,
                                     &frame->window_update);
    return 0;
  case NGHTTP2_CONTINUATION:
    /* We never handle CONTINUATION here. */
    assert(0);
    return 0;
  default: {
    nghttp2_ext_aux_data *aux_data;

    /* extension frame */

    aux_data = &item->aux_data.ext;

    if (aux_data->builtin == 0) {
      if (session_is_closing(session)) {
        return NGHTTP2_ERR_SESSION_CLOSING;
      }

      return session_pack_extension(session, &session->aob.framebufs, frame);
    }
      if (nghttp2_buf_mark_avail(&iframe->sbuf)) {
        return in - first;
      }

      if (iframe->sbuf.pos[3] != NGHTTP2_SETTINGS ||
          (iframe->sbuf.pos[4] & NGHTTP2_FLAG_ACK)) {
        rv = session_call_error_callback(
            session, NGHTTP2_ERR_SETTINGS_EXPECTED,
            "Remote peer returned unexpected data while we expected "
            "SETTINGS frame.  Perhaps, peer does not support HTTP/2 "
            "properly.");

        if (nghttp2_is_fatal(rv)) {
          return rv;
        }

        rv = nghttp2_session_terminate_session_with_reason(
            session, NGHTTP2_PROTOCOL_ERROR, "SETTINGS expected");

        if (nghttp2_is_fatal(rv)) {
          return rv;
        }

        return (ssize_t)inlen;
      }
      if (iframe->frame.hd.length > session->local_settings.max_frame_size) {
        DEBUGF("recv: length is too large %zu > %u\n", iframe->frame.hd.length,
               session->local_settings.max_frame_size);

        rv = nghttp2_session_terminate_session_with_reason(
            session, NGHTTP2_FRAME_SIZE_ERROR, "too large frame size");

        if (nghttp2_is_fatal(rv)) {
          return rv;
        }
      case NGHTTP2_DATA: {
        DEBUGF("recv: DATA\n");

        iframe->frame.hd.flags &=
            (NGHTTP2_FLAG_END_STREAM | NGHTTP2_FLAG_PADDED);
        /* Check stream is open. If it is not open or closing,
           ignore payload. */
        busy = 1;

        rv = session_on_data_received_fail_fast(session);
        if (iframe->state == NGHTTP2_IB_IGN_ALL) {
          return (ssize_t)inlen;
        }
        if (nghttp2_is_fatal(rv)) {
          return rv;
        }

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
          break;
        }

        iframe->state = NGHTTP2_IB_READ_DATA;
        break;
      }
      case NGHTTP2_HEADERS:

        DEBUGF("recv: HEADERS\n");

        iframe->frame.hd.flags &=
            (NGHTTP2_FLAG_END_STREAM | NGHTTP2_FLAG_END_HEADERS |
             NGHTTP2_FLAG_PADDED | NGHTTP2_FLAG_PRIORITY);

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
        if (nghttp2_is_fatal(rv)) {
          return rv;
        }

        busy = 1;

        if (iframe->state == NGHTTP2_IB_IGN_ALL) {
          return (ssize_t)inlen;
        }
          if (!iframe->iv) {
            return NGHTTP2_ERR_NOMEM;
          }

          min_header_table_size_entry = &iframe->iv[iframe->max_niv - 1];
          min_header_table_size_entry->settings_id =
              NGHTTP2_SETTINGS_HEADER_TABLE_SIZE;
          min_header_table_size_entry->value = UINT32_MAX;

          inbound_frame_set_mark(iframe, NGHTTP2_FRAME_SETTINGS_ENTRY_LENGTH);
          break;
        }

        busy = 1;

        inbound_frame_set_mark(iframe, 0);

        break;
      case NGHTTP2_PUSH_PROMISE:
        DEBUGF("recv: PUSH_PROMISE\n");

        iframe->frame.hd.flags &=
            (NGHTTP2_FLAG_END_HEADERS | NGHTTP2_FLAG_PADDED);

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
        if (nghttp2_is_fatal(rv)) {
          return rv;
        }

        return (ssize_t)inlen;
      default:
        DEBUGF("recv: extension frame\n");

        if (check_ext_type_set(session->user_recv_ext_types,
                               iframe->frame.hd.type)) {
          if (!session->callbacks.unpack_extension_callback) {
            /* Silently ignore unknown frame type. */

            busy = 1;

            iframe->state = NGHTTP2_IB_IGN_PAYLOAD;

            break;
          }
        switch (iframe->state) {
        case NGHTTP2_IB_IGN_HEADER_BLOCK:
        case NGHTTP2_IB_IGN_PAYLOAD:
        case NGHTTP2_IB_FRAME_SIZE_ERROR:
        case NGHTTP2_IB_IGN_DATA:
        case NGHTTP2_IB_IGN_ALL:
          break;
        default:
          rv = session_call_on_begin_frame(session, &iframe->frame.hd);

          if (nghttp2_is_fatal(rv)) {
            return rv;
          }
      switch (iframe->frame.hd.type) {
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
              break;
            }
            iframe->state = NGHTTP2_IB_READ_NBYTE;
            inbound_frame_set_mark(iframe, pri_fieldlen);
            break;
          } else {
            /* Truncate buffers used for padding spec */
            inbound_frame_set_mark(iframe, 0);
          }
        }

        rv = session_process_headers_frame(session);
        if (nghttp2_is_fatal(rv)) {
          return rv;
        }

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
          iframe->state = NGHTTP2_IB_IGN_HEADER_BLOCK;
          break;
        }

        if (rv == NGHTTP2_ERR_IGN_HEADER_BLOCK) {
          iframe->state = NGHTTP2_IB_IGN_HEADER_BLOCK;
          break;
        }

        iframe->state = NGHTTP2_IB_READ_HEADER_BLOCK;

        break;
      case NGHTTP2_PRIORITY:
        rv = session_process_priority_frame(session);
        if (nghttp2_is_fatal(rv)) {
          return rv;
        }

        if (iframe->state == NGHTTP2_IB_IGN_ALL) {
          return (ssize_t)inlen;
        }

        session_inbound_frame_reset(session);

        break;
      case NGHTTP2_RST_STREAM:
        rv = session_process_rst_stream_frame(session);
        if (nghttp2_is_fatal(rv)) {
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

          if (iframe->payloadleft < 4) {
            busy = 1;
            iframe->state = NGHTTP2_IB_FRAME_SIZE_ERROR;
            break;
          }

          iframe->state = NGHTTP2_IB_READ_NBYTE;

          inbound_frame_set_mark(iframe, 4);

          break;
        }

        rv = session_process_push_promise_frame(session);
        if (nghttp2_is_fatal(rv)) {
          return rv;
        }

        busy = 1;

        if (iframe->state == NGHTTP2_IB_IGN_ALL) {
          return (ssize_t)inlen;
        }

        if (rv == NGHTTP2_ERR_TEMPORAL_CALLBACK_FAILURE) {
          rv = nghttp2_session_add_rst_stream(
              session, iframe->frame.push_promise.promised_stream_id,
              NGHTTP2_INTERNAL_ERROR);
          if (nghttp2_is_fatal(rv)) {
            return rv;
          }
          iframe->state = NGHTTP2_IB_IGN_HEADER_BLOCK;
          break;
        }

        if (rv == NGHTTP2_ERR_IGN_HEADER_BLOCK) {
          iframe->state = NGHTTP2_IB_IGN_HEADER_BLOCK;
          break;
        }

        iframe->state = NGHTTP2_IB_READ_HEADER_BLOCK;

        break;
      case NGHTTP2_PING:
        rv = session_process_ping_frame(session);
        if (nghttp2_is_fatal(rv)) {
          return rv;
        }

        if (iframe->state == NGHTTP2_IB_IGN_ALL) {
          return (ssize_t)inlen;
        }

        session_inbound_frame_reset(session);

        break;
      case NGHTTP2_GOAWAY: {
        size_t debuglen;

        /* 8 is Last-stream-ID + Error Code */
        debuglen = iframe->frame.hd.length - 8;

        if (debuglen > 0) {
          iframe->raw_lbuf = nghttp2_mem_malloc(mem, debuglen);

          if (iframe->raw_lbuf == NULL) {
            return NGHTTP2_ERR_NOMEM;
          }

          nghttp2_buf_wrap_init(&iframe->lbuf, iframe->raw_lbuf, debuglen);
        }

        busy = 1;

        iframe->state = NGHTTP2_IB_READ_GOAWAY_DEBUG;

        break;
      }
      case NGHTTP2_WINDOW_UPDATE:
        rv = session_process_window_update_frame(session);
        if (nghttp2_is_fatal(rv)) {
          return rv;
        }

        if (iframe->state == NGHTTP2_IB_IGN_ALL) {
          return (ssize_t)inlen;
        }

        session_inbound_frame_reset(session);

        break;
      case NGHTTP2_ALTSVC: {
        size_t origin_len;

        origin_len = nghttp2_get_uint16(iframe->sbuf.pos);

        DEBUGF("recv: origin_len=%zu\n", origin_len);

        if (origin_len > iframe->payloadleft) {
          busy = 1;
          iframe->state = NGHTTP2_IB_FRAME_SIZE_ERROR;
          break;
        }

        if (iframe->frame.hd.length > 2) {
          iframe->raw_lbuf =
              nghttp2_mem_malloc(mem, iframe->frame.hd.length - 2);

          if (iframe->raw_lbuf == NULL) {
            return NGHTTP2_ERR_NOMEM;
          }

          nghttp2_buf_wrap_init(&iframe->lbuf, iframe->raw_lbuf,
                                iframe->frame.hd.length);
        }

        busy = 1;

        iframe->state = NGHTTP2_IB_READ_ALTSVC_PAYLOAD;

        break;
      }
      default:
        /* This is unknown frame */
        session_inbound_frame_reset(session);

        break;
      }
        if (nghttp2_is_fatal(rv)) {
          return rv;
        }

        busy = 1;

        if (iframe->state == NGHTTP2_IB_IGN_ALL) {
          return (ssize_t)inlen;
        }
        if (nghttp2_is_fatal(rv)) {
          return rv;
        }

        if (iframe->state == NGHTTP2_IB_IGN_ALL) {
          return (ssize_t)inlen;
        }

        session_inbound_frame_reset(session);

        break;
      case NGHTTP2_RST_STREAM:
        rv = session_process_rst_stream_frame(session);
        if (nghttp2_is_fatal(rv)) {
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
        if (nghttp2_is_fatal(rv)) {
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
        if (nghttp2_is_fatal(rv)) {
          return rv;
        }

        busy = 1;

        if (iframe->state == NGHTTP2_IB_IGN_ALL) {
          return (ssize_t)inlen;
        }
        if (nghttp2_is_fatal(rv)) {
          return rv;
        }

        if (iframe->state == NGHTTP2_IB_IGN_ALL) {
          return (ssize_t)inlen;
        }

        session_inbound_frame_reset(session);

        break;
      case NGHTTP2_GOAWAY: {
        size_t debuglen;

        /* 8 is Last-stream-ID + Error Code */
        debuglen = iframe->frame.hd.length - 8;

        if (debuglen > 0) {
          iframe->raw_lbuf = nghttp2_mem_malloc(mem, debuglen);

          if (iframe->raw_lbuf == NULL) {
            return NGHTTP2_ERR_NOMEM;
          }

          nghttp2_buf_wrap_init(&iframe->lbuf, iframe->raw_lbuf, debuglen);
        }
        if (nghttp2_is_fatal(rv)) {
          return rv;
        }

        if (iframe->state == NGHTTP2_IB_IGN_ALL) {
          return (ssize_t)inlen;
        }

        session_inbound_frame_reset(session);

        break;
      case NGHTTP2_ALTSVC: {
        size_t origin_len;

        origin_len = nghttp2_get_uint16(iframe->sbuf.pos);

        DEBUGF("recv: origin_len=%zu\n", origin_len);

        if (origin_len > iframe->payloadleft) {
          busy = 1;
          iframe->state = NGHTTP2_IB_FRAME_SIZE_ERROR;
          break;
        }
      } else {
        DEBUGF("recv: [IB_IGN_HEADER_BLOCK]\n");
      }
#endif /* DEBUGBUILD */

      readlen = inbound_frame_payload_readlen(iframe, in, last);

      DEBUGF("recv: readlen=%zu, payloadleft=%zu\n", readlen,
             iframe->payloadleft - readlen);

      data_readlen = inbound_frame_effective_readlen(
          iframe, iframe->payloadleft - readlen, readlen);

      if (data_readlen == -1) {
        /* everything is padding */
        data_readlen = 0;
      }
        if (nghttp2_is_fatal(rv)) {
          return rv;
        }

        if (iframe->state == NGHTTP2_IB_IGN_ALL) {
          return (ssize_t)inlen;
        }

        if (rv == NGHTTP2_ERR_PAUSE) {
          in += hd_proclen;
          iframe->payloadleft -= hd_proclen;

          return in - first;
        }

        if (rv == NGHTTP2_ERR_TEMPORAL_CALLBACK_FAILURE) {
          /* The application says no more headers. We decompress the
             rest of the header block but not invoke on_header_callback
             and on_frame_recv_callback. */
          in += hd_proclen;
          iframe->payloadleft -= hd_proclen;

          /* Use promised stream ID for PUSH_PROMISE */
          rv = nghttp2_session_add_rst_stream(
              session,
              iframe->frame.hd.type == NGHTTP2_PUSH_PROMISE
                  ? iframe->frame.push_promise.promised_stream_id
                  : iframe->frame.hd.stream_id,
              NGHTTP2_INTERNAL_ERROR);
          if (nghttp2_is_fatal(rv)) {
            return rv;
          }
      if (nghttp2_is_fatal(rv)) {
        return rv;
      }

      assert(iframe->state == NGHTTP2_IB_IGN_ALL);

      return (ssize_t)inlen;
    case NGHTTP2_IB_READ_SETTINGS:
      DEBUGF("recv: [IB_READ_SETTINGS]\n");

      readlen = inbound_frame_buf_read(iframe, in, last);
      iframe->payloadleft -= readlen;
      in += readlen;

      DEBUGF("recv: readlen=%zu, payloadleft=%zu\n", readlen,
             iframe->payloadleft);

      if (nghttp2_buf_mark_avail(&iframe->sbuf)) {
        break;
      }

      if (readlen > 0) {
        inbound_frame_set_settings_entry(iframe);
      }
      if (iframe->payloadleft) {
        inbound_frame_set_mark(iframe, NGHTTP2_FRAME_SETTINGS_ENTRY_LENGTH);
        break;
      }

      rv = session_process_settings_frame(session);

      if (nghttp2_is_fatal(rv)) {
        return rv;
      }

      if (iframe->state == NGHTTP2_IB_IGN_ALL) {
        return (ssize_t)inlen;
      }

      session_inbound_frame_reset(session);

      break;
    case NGHTTP2_IB_READ_GOAWAY_DEBUG:
      DEBUGF("recv: [IB_READ_GOAWAY_DEBUG]\n");

      readlen = inbound_frame_payload_readlen(iframe, in, last);

      if (readlen > 0) {
        iframe->lbuf.last = nghttp2_cpymem(iframe->lbuf.last, in, readlen);

        iframe->payloadleft -= readlen;
        in += readlen;
      }

      DEBUGF("recv: readlen=%zu, payloadleft=%zu\n", readlen,
             iframe->payloadleft);

      if (iframe->payloadleft) {
        assert(nghttp2_buf_avail(&iframe->lbuf) > 0);

        break;
      }

      rv = session_process_goaway_frame(session);

      if (nghttp2_is_fatal(rv)) {
        return rv;
      }

      if (iframe->state == NGHTTP2_IB_IGN_ALL) {
        return (ssize_t)inlen;
      }

      session_inbound_frame_reset(session);

      break;
    case NGHTTP2_IB_EXPECT_CONTINUATION:
    case NGHTTP2_IB_IGN_CONTINUATION:
#ifdef DEBUGBUILD
      if (iframe->state == NGHTTP2_IB_EXPECT_CONTINUATION) {
        fprintf(stderr, "recv: [IB_EXPECT_CONTINUATION]\n");
      } else {
        fprintf(stderr, "recv: [IB_IGN_CONTINUATION]\n");
      }
      if (nghttp2_is_fatal(rv)) {
        return rv;
      }

      if (iframe->state == NGHTTP2_IB_IGN_ALL) {
        return (ssize_t)inlen;
      }

      session_inbound_frame_reset(session);

      break;
    case NGHTTP2_IB_READ_GOAWAY_DEBUG:
      DEBUGF("recv: [IB_READ_GOAWAY_DEBUG]\n");

      readlen = inbound_frame_payload_readlen(iframe, in, last);

      if (readlen > 0) {
        iframe->lbuf.last = nghttp2_cpymem(iframe->lbuf.last, in, readlen);

        iframe->payloadleft -= readlen;
        in += readlen;
      }

      DEBUGF("recv: readlen=%zu, payloadleft=%zu\n", readlen,
             iframe->payloadleft);

      if (iframe->payloadleft) {
        assert(nghttp2_buf_avail(&iframe->lbuf) > 0);

        break;
      }

      rv = session_process_goaway_frame(session);

      if (nghttp2_is_fatal(rv)) {
        return rv;
      }

      if (iframe->state == NGHTTP2_IB_IGN_ALL) {
        return (ssize_t)inlen;
      }

      session_inbound_frame_reset(session);

      break;
    case NGHTTP2_IB_EXPECT_CONTINUATION:
    case NGHTTP2_IB_IGN_CONTINUATION:
#ifdef DEBUGBUILD
      if (iframe->state == NGHTTP2_IB_EXPECT_CONTINUATION) {
        fprintf(stderr, "recv: [IB_EXPECT_CONTINUATION]\n");
      } else {
        fprintf(stderr, "recv: [IB_IGN_CONTINUATION]\n");
      }
      if (nghttp2_is_fatal(rv)) {
        return rv;
      }

      if (iframe->state == NGHTTP2_IB_IGN_ALL) {
        return (ssize_t)inlen;
      }

      session_inbound_frame_reset(session);

      break;
    case NGHTTP2_IB_EXPECT_CONTINUATION:
    case NGHTTP2_IB_IGN_CONTINUATION:
#ifdef DEBUGBUILD
      if (iframe->state == NGHTTP2_IB_EXPECT_CONTINUATION) {
        fprintf(stderr, "recv: [IB_EXPECT_CONTINUATION]\n");
      } else {
        fprintf(stderr, "recv: [IB_IGN_CONTINUATION]\n");
      }
        if (nghttp2_is_fatal(rv)) {
          return rv;
        }

        return (ssize_t)inlen;
      }

      /* CONTINUATION won't bear NGHTTP2_PADDED flag */

      iframe->frame.hd.flags = (uint8_t)(
          iframe->frame.hd.flags | (cont_hd.flags & NGHTTP2_FLAG_END_HEADERS));
      iframe->frame.hd.length += cont_hd.length;

      busy = 1;

      if (iframe->state == NGHTTP2_IB_EXPECT_CONTINUATION) {
        iframe->state = NGHTTP2_IB_READ_HEADER_BLOCK;

        rv = session_call_on_begin_frame(session, &cont_hd);

        if (nghttp2_is_fatal(rv)) {
          return rv;
        }
      } else {
        iframe->state = NGHTTP2_IB_IGN_HEADER_BLOCK;
      }

      break;
    case NGHTTP2_IB_READ_PAD_DATA:
      DEBUGF("recv: [IB_READ_PAD_DATA]\n");

      readlen = inbound_frame_buf_read(iframe, in, last);
      in += readlen;
      iframe->payloadleft -= readlen;

      DEBUGF("recv: readlen=%zu, payloadleft=%zu, left=%zu\n", readlen,
             iframe->payloadleft, nghttp2_buf_mark_avail(&iframe->sbuf));

      if (nghttp2_buf_mark_avail(&iframe->sbuf)) {
        return in - first;
      }

      /* Pad Length field is subject to flow control */
      rv = session_update_recv_connection_window_size(session, readlen);
      if (nghttp2_is_fatal(rv)) {
        return rv;
      }

      if (iframe->state == NGHTTP2_IB_IGN_ALL) {
        return (ssize_t)inlen;
      }

      /* Pad Length field is consumed immediately */
      rv =
          nghttp2_session_consume(session, iframe->frame.hd.stream_id, readlen);

      if (nghttp2_is_fatal(rv)) {
        return rv;
      }

      if (iframe->state == NGHTTP2_IB_IGN_ALL) {
        return (ssize_t)inlen;
      }

      stream = nghttp2_session_get_stream(session, iframe->frame.hd.stream_id);
      if (stream) {
        rv = session_update_recv_stream_window_size(
            session, stream, readlen,
            iframe->payloadleft ||
                (iframe->frame.hd.flags & NGHTTP2_FLAG_END_STREAM) == 0);
        if (nghttp2_is_fatal(rv)) {
          return rv;
        }
      if (nghttp2_is_fatal(rv)) {
        return rv;
      }

      if (iframe->state == NGHTTP2_IB_IGN_ALL) {
        return (ssize_t)inlen;
      }

      /* Pad Length field is consumed immediately */
      rv =
          nghttp2_session_consume(session, iframe->frame.hd.stream_id, readlen);

      if (nghttp2_is_fatal(rv)) {
        return rv;
      }

      if (iframe->state == NGHTTP2_IB_IGN_ALL) {
        return (ssize_t)inlen;
      }

      stream = nghttp2_session_get_stream(session, iframe->frame.hd.stream_id);
      if (stream) {
        rv = session_update_recv_stream_window_size(
            session, stream, readlen,
            iframe->payloadleft ||
                (iframe->frame.hd.flags & NGHTTP2_FLAG_END_STREAM) == 0);
        if (nghttp2_is_fatal(rv)) {
          return rv;
        }
      if (nghttp2_is_fatal(rv)) {
        return rv;
      }

      if (iframe->state == NGHTTP2_IB_IGN_ALL) {
        return (ssize_t)inlen;
      }

      stream = nghttp2_session_get_stream(session, iframe->frame.hd.stream_id);
      if (stream) {
        rv = session_update_recv_stream_window_size(
            session, stream, readlen,
            iframe->payloadleft ||
                (iframe->frame.hd.flags & NGHTTP2_FLAG_END_STREAM) == 0);
        if (nghttp2_is_fatal(rv)) {
          return rv;
        }
        if (nghttp2_is_fatal(rv)) {
          return rv;
        }
        return (ssize_t)inlen;
      }

      iframe->frame.data.padlen = (size_t)padlen;

      iframe->state = NGHTTP2_IB_READ_DATA;

      break;
    case NGHTTP2_IB_READ_DATA:
      stream = nghttp2_session_get_stream(session, iframe->frame.hd.stream_id);

      if (!stream) {
        busy = 1;
        iframe->state = NGHTTP2_IB_IGN_DATA;
        break;
      }

      DEBUGF("recv: [IB_READ_DATA]\n");

      readlen = inbound_frame_payload_readlen(iframe, in, last);
      iframe->payloadleft -= readlen;
      in += readlen;

      DEBUGF("recv: readlen=%zu, payloadleft=%zu\n", readlen,
             iframe->payloadleft);

      if (readlen > 0) {
        ssize_t data_readlen;

        rv = session_update_recv_connection_window_size(session, readlen);
        if (nghttp2_is_fatal(rv)) {
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

        data_readlen = inbound_frame_effective_readlen(
            iframe, iframe->payloadleft, readlen);

        if (data_readlen == -1) {
          /* everything is padding */
          data_readlen = 0;
        }

        padlen = (ssize_t)readlen - data_readlen;

        if (padlen > 0) {
          /* Padding is considered as "consumed" immediately */
          rv = nghttp2_session_consume(session, iframe->frame.hd.stream_id,
                                       (size_t)padlen);

          if (nghttp2_is_fatal(rv)) {
            return rv;
          }
        if (nghttp2_is_fatal(rv)) {
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

        data_readlen = inbound_frame_effective_readlen(
            iframe, iframe->payloadleft, readlen);

        if (data_readlen == -1) {
          /* everything is padding */
          data_readlen = 0;
        }

        padlen = (ssize_t)readlen - data_readlen;

        if (padlen > 0) {
          /* Padding is considered as "consumed" immediately */
          rv = nghttp2_session_consume(session, iframe->frame.hd.stream_id,
                                       (size_t)padlen);

          if (nghttp2_is_fatal(rv)) {
            return rv;
          }
          if (nghttp2_is_fatal(rv)) {
            return rv;
          }

          if (iframe->state == NGHTTP2_IB_IGN_ALL) {
            return (ssize_t)inlen;
          }
        }

        DEBUGF("recv: data_readlen=%zd\n", data_readlen);

        if (data_readlen > 0) {
          if (session_enforce_http_messaging(session)) {
            if (nghttp2_http_on_data_chunk(stream, (size_t)data_readlen) != 0) {
              if (session->opt_flags & NGHTTP2_OPTMASK_NO_AUTO_WINDOW_UPDATE) {
                /* Consume all data for connection immediately here */
                rv = session_update_connection_consumed_size(
                    session, (size_t)data_readlen);

                if (nghttp2_is_fatal(rv)) {
                  return rv;
                }

                if (iframe->state == NGHTTP2_IB_IGN_DATA) {
                  return (ssize_t)inlen;
                }
              }

              rv = nghttp2_session_add_rst_stream(
                  session, iframe->frame.hd.stream_id, NGHTTP2_PROTOCOL_ERROR);
              if (nghttp2_is_fatal(rv)) {
                return rv;
              }
              busy = 1;
              iframe->state = NGHTTP2_IB_IGN_DATA;
              break;
            }
                if (nghttp2_is_fatal(rv)) {
                  return rv;
                }

                if (iframe->state == NGHTTP2_IB_IGN_DATA) {
                  return (ssize_t)inlen;
                }
              }

              rv = nghttp2_session_add_rst_stream(
                  session, iframe->frame.hd.stream_id, NGHTTP2_PROTOCOL_ERROR);
              if (nghttp2_is_fatal(rv)) {
                return rv;
              }
              busy = 1;
              iframe->state = NGHTTP2_IB_IGN_DATA;
              break;
            }
          }
          if (session->callbacks.on_data_chunk_recv_callback) {
            rv = session->callbacks.on_data_chunk_recv_callback(
                session, iframe->frame.hd.flags, iframe->frame.hd.stream_id,
                in - readlen, (size_t)data_readlen, session->user_data);
            if (rv == NGHTTP2_ERR_PAUSE) {
              return in - first;
            }

            if (nghttp2_is_fatal(rv)) {
              return NGHTTP2_ERR_CALLBACK_FAILURE;
            }
          }
        }
      }

      if (iframe->payloadleft) {
        break;
      }

      rv = session_process_data_frame(session);
      if (nghttp2_is_fatal(rv)) {
        return rv;
      }

      session_inbound_frame_reset(session);

      break;
    case NGHTTP2_IB_IGN_DATA:
      DEBUGF("recv: [IB_IGN_DATA]\n");

      readlen = inbound_frame_payload_readlen(iframe, in, last);
      iframe->payloadleft -= readlen;
      in += readlen;

      DEBUGF("recv: readlen=%zu, payloadleft=%zu\n", readlen,
             iframe->payloadleft);

      if (readlen > 0) {
        /* Update connection-level flow control window for ignored
           DATA frame too */
        rv = session_update_recv_connection_window_size(session, readlen);
        if (nghttp2_is_fatal(rv)) {
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
        break;
      }

      session_inbound_frame_reset(session);

      break;
    case NGHTTP2_IB_IGN_ALL:
      return (ssize_t)inlen;
    case NGHTTP2_IB_READ_EXTENSION_PAYLOAD:
      DEBUGF("recv: [IB_READ_EXTENSION_PAYLOAD]\n");

      readlen = inbound_frame_payload_readlen(iframe, in, last);
      iframe->payloadleft -= readlen;
      in += readlen;

      DEBUGF("recv: readlen=%zu, payloadleft=%zu\n", readlen,
             iframe->payloadleft);

      if (readlen > 0) {
        rv = session_call_on_extension_chunk_recv_callback(
            session, in - readlen, readlen);
        if (nghttp2_is_fatal(rv)) {
          return rv;
        }

        if (rv != 0) {
          busy = 1;

          iframe->state = NGHTTP2_IB_IGN_PAYLOAD;

          break;
        }
      }

      if (iframe->payloadleft > 0) {
        break;
      }

      rv = session_process_extension_frame(session);
      if (nghttp2_is_fatal(rv)) {
        return rv;
      }

      session_inbound_frame_reset(session);

      break;
    case NGHTTP2_IB_READ_ALTSVC_PAYLOAD:
      DEBUGF("recv: [IB_READ_ALTSVC_PAYLOAD]\n");

      readlen = inbound_frame_payload_readlen(iframe, in, last);

      if (readlen > 0) {
        iframe->lbuf.last = nghttp2_cpymem(iframe->lbuf.last, in, readlen);

        iframe->payloadleft -= readlen;
        in += readlen;
      }

      DEBUGF("recv: readlen=%zu, payloadleft=%zu\n", readlen,
             iframe->payloadleft);

      if (iframe->payloadleft) {
        assert(nghttp2_buf_avail(&iframe->lbuf) > 0);

        break;
      }

      rv = session_process_altsvc_frame(session);

      if (nghttp2_is_fatal(rv)) {
        return rv;
      }

      session_inbound_frame_reset(session);

      break;
    }

    if (!busy && in == last) {
      break;
    }

    busy = 0;
  }

  assert(in == last);

  return in - first;
}

int nghttp2_session_recv(nghttp2_session *session) {
  uint8_t buf[NGHTTP2_INBOUND_BUFFER_LENGTH];
  while (1) {
    ssize_t readlen;
    readlen = session_recv(session, buf, sizeof(buf));
    if (readlen > 0) {
      ssize_t proclen = nghttp2_session_mem_recv(session, buf, (size_t)readlen);
      if (proclen < 0) {
        return (int)proclen;
      }
      assert(proclen == readlen);
    } else if (readlen == 0 || readlen == NGHTTP2_ERR_WOULDBLOCK) {
      return 0;
    } else if (readlen == NGHTTP2_ERR_EOF) {
      return NGHTTP2_ERR_EOF;
    } else if (readlen < 0) {
      return NGHTTP2_ERR_CALLBACK_FAILURE;
    }
  }
}

/*
 * Returns the number of active streams, which includes streams in
 * reserved state.
 */
static size_t session_get_num_active_streams(nghttp2_session *session) {
  return nghttp2_map_size(&session->streams) - session->num_closed_streams -
         session->num_idle_streams;
}

int nghttp2_session_want_read(nghttp2_session *session) {
  size_t num_active_streams;

  /* If this flag is set, we don't want to read. The application
     should drop the connection. */
  if (session->goaway_flags & NGHTTP2_GOAWAY_TERM_SENT) {
    return 0;
  }

  num_active_streams = session_get_num_active_streams(session);

  /* Unless termination GOAWAY is sent or received, we always want to
     read incoming frames. */

  if (num_active_streams > 0) {
    return 1;
  }

  /* If there is no active streams and GOAWAY has been sent or
     received, we are done with this session. */
  return (session->goaway_flags &
          (NGHTTP2_GOAWAY_SENT | NGHTTP2_GOAWAY_RECV)) == 0;
}

int nghttp2_session_want_write(nghttp2_session *session) {
  /* If these flag is set, we don't want to write any data. The
     application should drop the connection. */
  if (session->goaway_flags & NGHTTP2_GOAWAY_TERM_SENT) {
    return 0;
  }

  /*
   * Unless termination GOAWAY is sent or received, we want to write
   * frames if there is pending ones. If pending frame is request/push
   * response HEADERS and concurrent stream limit is reached, we don't
   * want to write them.
   */
  return session->aob.item || nghttp2_outbound_queue_top(&session->ob_urgent) ||
         nghttp2_outbound_queue_top(&session->ob_reg) ||
         (!nghttp2_pq_empty(&session->root.obq) &&
          session->remote_window_size > 0) ||
         (nghttp2_outbound_queue_top(&session->ob_syn) &&
          !session_is_outgoing_concurrent_streams_max(session));
}

int nghttp2_session_add_ping(nghttp2_session *session, uint8_t flags,
                             const uint8_t *opaque_data) {
  int rv;
  nghttp2_outbound_item *item;
  nghttp2_frame *frame;
  nghttp2_mem *mem;

  mem = &session->mem;

  if ((flags & NGHTTP2_FLAG_ACK) &&
      session->obq_flood_counter_ >= NGHTTP2_MAX_OBQ_FLOOD_ITEM) {
    return NGHTTP2_ERR_FLOODED;
  }

  item = nghttp2_mem_malloc(mem, sizeof(nghttp2_outbound_item));
  if (item == NULL) {
    return NGHTTP2_ERR_NOMEM;
  }

  nghttp2_outbound_item_init(item);

  frame = &item->frame;

  nghttp2_frame_ping_init(&frame->ping, flags, opaque_data);

  rv = nghttp2_session_add_item(session, item);

  if (rv != 0) {
    nghttp2_frame_ping_free(&frame->ping);
    nghttp2_mem_free(mem, item);
    return rv;
  }

  if (flags & NGHTTP2_FLAG_ACK) {
    ++session->obq_flood_counter_;
  }

  return 0;
}

int nghttp2_session_add_goaway(nghttp2_session *session, int32_t last_stream_id,
                               uint32_t error_code, const uint8_t *opaque_data,
                               size_t opaque_data_len, uint8_t aux_flags) {
  int rv;
  nghttp2_outbound_item *item;
  nghttp2_frame *frame;
  uint8_t *opaque_data_copy = NULL;
  nghttp2_goaway_aux_data *aux_data;
  nghttp2_mem *mem;

  mem = &session->mem;

  if (nghttp2_session_is_my_stream_id(session, last_stream_id)) {
    return NGHTTP2_ERR_INVALID_ARGUMENT;
  }

  if (opaque_data_len) {
    if (opaque_data_len + 8 > NGHTTP2_MAX_PAYLOADLEN) {
      return NGHTTP2_ERR_INVALID_ARGUMENT;
    }
    opaque_data_copy = nghttp2_mem_malloc(mem, opaque_data_len);
    if (opaque_data_copy == NULL) {
      return NGHTTP2_ERR_NOMEM;
    }
    memcpy(opaque_data_copy, opaque_data, opaque_data_len);
  }

  item = nghttp2_mem_malloc(mem, sizeof(nghttp2_outbound_item));
  if (item == NULL) {
    nghttp2_mem_free(mem, opaque_data_copy);
    return NGHTTP2_ERR_NOMEM;
  }

  nghttp2_outbound_item_init(item);

  frame = &item->frame;

  /* last_stream_id must not be increased from the value previously
     sent */
  last_stream_id = nghttp2_min(last_stream_id, session->local_last_stream_id);

  nghttp2_frame_goaway_init(&frame->goaway, last_stream_id, error_code,
                            opaque_data_copy, opaque_data_len);

  aux_data = &item->aux_data.goaway;
  aux_data->flags = aux_flags;

  rv = nghttp2_session_add_item(session, item);
  if (rv != 0) {
    nghttp2_frame_goaway_free(&frame->goaway, mem);
    nghttp2_mem_free(mem, item);
    return rv;
  }
  return 0;
}

int nghttp2_session_add_window_update(nghttp2_session *session, uint8_t flags,
                                      int32_t stream_id,
                                      int32_t window_size_increment) {
  int rv;
  nghttp2_outbound_item *item;
  nghttp2_frame *frame;
  nghttp2_mem *mem;

  mem = &session->mem;
  item = nghttp2_mem_malloc(mem, sizeof(nghttp2_outbound_item));
  if (item == NULL) {
    return NGHTTP2_ERR_NOMEM;
  }

  nghttp2_outbound_item_init(item);

  frame = &item->frame;

  nghttp2_frame_window_update_init(&frame->window_update, flags, stream_id,
                                   window_size_increment);

  rv = nghttp2_session_add_item(session, item);

  if (rv != 0) {
    nghttp2_frame_window_update_free(&frame->window_update);
    nghttp2_mem_free(mem, item);
    return rv;
  }
  return 0;
}

static void
session_append_inflight_settings(nghttp2_session *session,
                                 nghttp2_inflight_settings *settings) {
  nghttp2_inflight_settings **i;

  for (i = &session->inflight_settings_head; *i; i = &(*i)->next)
    ;

  *i = settings;
}

int nghttp2_session_add_settings(nghttp2_session *session, uint8_t flags,
                                 const nghttp2_settings_entry *iv, size_t niv) {
  nghttp2_outbound_item *item;
  nghttp2_frame *frame;
  nghttp2_settings_entry *iv_copy;
  size_t i;
  int rv;
  nghttp2_mem *mem;
  nghttp2_inflight_settings *inflight_settings = NULL;

  mem = &session->mem;

  if (flags & NGHTTP2_FLAG_ACK) {
    if (niv != 0) {
      return NGHTTP2_ERR_INVALID_ARGUMENT;
    }

    if (session->obq_flood_counter_ >= NGHTTP2_MAX_OBQ_FLOOD_ITEM) {
      return NGHTTP2_ERR_FLOODED;
    }
  }

  if (!nghttp2_iv_check(iv, niv)) {
    return NGHTTP2_ERR_INVALID_ARGUMENT;
  }

  item = nghttp2_mem_malloc(mem, sizeof(nghttp2_outbound_item));
  if (item == NULL) {
    return NGHTTP2_ERR_NOMEM;
  }

  if (niv > 0) {
    iv_copy = nghttp2_frame_iv_copy(iv, niv, mem);
    if (iv_copy == NULL) {
      nghttp2_mem_free(mem, item);
      return NGHTTP2_ERR_NOMEM;
    }
  } else {
    iv_copy = NULL;
  }

  if ((flags & NGHTTP2_FLAG_ACK) == 0) {
    rv = inflight_settings_new(&inflight_settings, iv, niv, mem);
    if (rv != 0) {
      assert(nghttp2_is_fatal(rv));
      nghttp2_mem_free(mem, iv_copy);
      nghttp2_mem_free(mem, item);
      return rv;
    }
  }

  nghttp2_outbound_item_init(item);

  frame = &item->frame;

  nghttp2_frame_settings_init(&frame->settings, flags, iv_copy, niv);
  rv = nghttp2_session_add_item(session, item);
  if (rv != 0) {
    /* The only expected error is fatal one */
    assert(nghttp2_is_fatal(rv));

    inflight_settings_del(inflight_settings, mem);

    nghttp2_frame_settings_free(&frame->settings, mem);
    nghttp2_mem_free(mem, item);

    return rv;
  }

  if (flags & NGHTTP2_FLAG_ACK) {
    ++session->obq_flood_counter_;
  } else {
    session_append_inflight_settings(session, inflight_settings);
  }

  /* Extract NGHTTP2_SETTINGS_MAX_CONCURRENT_STREAMS and ENABLE_PUSH
     here.  We use it to refuse the incoming stream and PUSH_PROMISE
     with RST_STREAM. */

  for (i = niv; i > 0; --i) {
    if (iv[i - 1].settings_id == NGHTTP2_SETTINGS_MAX_CONCURRENT_STREAMS) {
      session->pending_local_max_concurrent_stream = iv[i - 1].value;
      break;
    }
  }

  for (i = niv; i > 0; --i) {
    if (iv[i - 1].settings_id == NGHTTP2_SETTINGS_ENABLE_PUSH) {
      session->pending_enable_push = (uint8_t)iv[i - 1].value;
      break;
    }
  }

  return 0;
}

int nghttp2_session_pack_data(nghttp2_session *session, nghttp2_bufs *bufs,
                              size_t datamax, nghttp2_frame *frame,
                              nghttp2_data_aux_data *aux_data,
                              nghttp2_stream *stream) {
  int rv;
  uint32_t data_flags;
  ssize_t payloadlen;
  ssize_t padded_payloadlen;
  nghttp2_buf *buf;
  size_t max_payloadlen;

  assert(bufs->head == bufs->cur);

  buf = &bufs->cur->buf;

  if (session->callbacks.read_length_callback) {

    payloadlen = session->callbacks.read_length_callback(
        session, frame->hd.type, stream->stream_id, session->remote_window_size,
        stream->remote_window_size, session->remote_settings.max_frame_size,
        session->user_data);

    DEBUGF("send: read_length_callback=%zd\n", payloadlen);

    payloadlen = nghttp2_session_enforce_flow_control_limits(session, stream,
                                                             payloadlen);

    DEBUGF("send: read_length_callback after flow control=%zd\n", payloadlen);

    if (payloadlen <= 0) {
      return NGHTTP2_ERR_CALLBACK_FAILURE;
    }

    if ((size_t)payloadlen > nghttp2_buf_avail(buf)) {
      /* Resize the current buffer(s).  The reason why we do +1 for
         buffer size is for possible padding field. */
      rv = nghttp2_bufs_realloc(&session->aob.framebufs,
                                (size_t)(NGHTTP2_FRAME_HDLEN + 1 + payloadlen));

      if (rv != 0) {
        DEBUGF("send: realloc buffer failed rv=%d", rv);
        /* If reallocation failed, old buffers are still in tact.  So
           use safe limit. */
        payloadlen = (ssize_t)datamax;

        DEBUGF("send: use safe limit payloadlen=%zd", payloadlen);
      } else {
        assert(&session->aob.framebufs == bufs);

        buf = &bufs->cur->buf;
      }
        if (nghttp2_is_fatal(rv)) {
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
          if (nghttp2_is_fatal(rv)) {
            return rv;
          }

          if (iframe->state == NGHTTP2_IB_IGN_ALL) {
            return (ssize_t)inlen;
          }
        }
      }

      if (iframe->payloadleft) {
        break;
      }

      session_inbound_frame_reset(session);

      break;
    case NGHTTP2_IB_IGN_ALL:
      return (ssize_t)inlen;
    case NGHTTP2_IB_READ_EXTENSION_PAYLOAD:
      DEBUGF("recv: [IB_READ_EXTENSION_PAYLOAD]\n");

      readlen = inbound_frame_payload_readlen(iframe, in, last);
      iframe->payloadleft -= readlen;
      in += readlen;

      DEBUGF("recv: readlen=%zu, payloadleft=%zu\n", readlen,
             iframe->payloadleft);

      if (readlen > 0) {
        rv = session_call_on_extension_chunk_recv_callback(
            session, in - readlen, readlen);
        if (nghttp2_is_fatal(rv)) {
          return rv;
        }

        if (rv != 0) {
          busy = 1;

          iframe->state = NGHTTP2_IB_IGN_PAYLOAD;

          break;
        }
      }

      if (iframe->payloadleft > 0) {
        break;
      }

      rv = session_process_extension_frame(session);
      if (nghttp2_is_fatal(rv)) {
        return rv;
      }

      session_inbound_frame_reset(session);

      break;
    case NGHTTP2_IB_READ_ALTSVC_PAYLOAD:
      DEBUGF("recv: [IB_READ_ALTSVC_PAYLOAD]\n");

      readlen = inbound_frame_payload_readlen(iframe, in, last);

      if (readlen > 0) {
        iframe->lbuf.last = nghttp2_cpymem(iframe->lbuf.last, in, readlen);

        iframe->payloadleft -= readlen;
        in += readlen;
      }

      DEBUGF("recv: readlen=%zu, payloadleft=%zu\n", readlen,
             iframe->payloadleft);

      if (iframe->payloadleft) {
        assert(nghttp2_buf_avail(&iframe->lbuf) > 0);

        break;
      }

      rv = session_process_altsvc_frame(session);

      if (nghttp2_is_fatal(rv)) {
        return rv;
      }

      session_inbound_frame_reset(session);

      break;
    }

    if (!busy && in == last) {
      break;
    }

    busy = 0;
  }

  assert(in == last);

  return in - first;
}

int nghttp2_session_recv(nghttp2_session *session) {
  uint8_t buf[NGHTTP2_INBOUND_BUFFER_LENGTH];
  while (1) {
    ssize_t readlen;
    readlen = session_recv(session, buf, sizeof(buf));
    if (readlen > 0) {
      ssize_t proclen = nghttp2_session_mem_recv(session, buf, (size_t)readlen);
      if (proclen < 0) {
        return (int)proclen;
      }
      assert(proclen == readlen);
    } else if (readlen == 0 || readlen == NGHTTP2_ERR_WOULDBLOCK) {
      return 0;
    } else if (readlen == NGHTTP2_ERR_EOF) {
      return NGHTTP2_ERR_EOF;
    } else if (readlen < 0) {
      return NGHTTP2_ERR_CALLBACK_FAILURE;
    }
  }
}

/*
 * Returns the number of active streams, which includes streams in
 * reserved state.
 */
static size_t session_get_num_active_streams(nghttp2_session *session) {
  return nghttp2_map_size(&session->streams) - session->num_closed_streams -
         session->num_idle_streams;
}

int nghttp2_session_want_read(nghttp2_session *session) {
  size_t num_active_streams;

  /* If this flag is set, we don't want to read. The application
     should drop the connection. */
  if (session->goaway_flags & NGHTTP2_GOAWAY_TERM_SENT) {
    return 0;
  }

  num_active_streams = session_get_num_active_streams(session);

  /* Unless termination GOAWAY is sent or received, we always want to
     read incoming frames. */

  if (num_active_streams > 0) {
    return 1;
  }

  /* If there is no active streams and GOAWAY has been sent or
     received, we are done with this session. */
  return (session->goaway_flags &
          (NGHTTP2_GOAWAY_SENT | NGHTTP2_GOAWAY_RECV)) == 0;
}

int nghttp2_session_want_write(nghttp2_session *session) {
  /* If these flag is set, we don't want to write any data. The
     application should drop the connection. */
  if (session->goaway_flags & NGHTTP2_GOAWAY_TERM_SENT) {
    return 0;
  }

  /*
   * Unless termination GOAWAY is sent or received, we want to write
   * frames if there is pending ones. If pending frame is request/push
   * response HEADERS and concurrent stream limit is reached, we don't
   * want to write them.
   */
  return session->aob.item || nghttp2_outbound_queue_top(&session->ob_urgent) ||
         nghttp2_outbound_queue_top(&session->ob_reg) ||
         (!nghttp2_pq_empty(&session->root.obq) &&
          session->remote_window_size > 0) ||
         (nghttp2_outbound_queue_top(&session->ob_syn) &&
          !session_is_outgoing_concurrent_streams_max(session));
}

int nghttp2_session_add_ping(nghttp2_session *session, uint8_t flags,
                             const uint8_t *opaque_data) {
  int rv;
  nghttp2_outbound_item *item;
  nghttp2_frame *frame;
  nghttp2_mem *mem;

  mem = &session->mem;

  if ((flags & NGHTTP2_FLAG_ACK) &&
      session->obq_flood_counter_ >= NGHTTP2_MAX_OBQ_FLOOD_ITEM) {
    return NGHTTP2_ERR_FLOODED;
  }

  item = nghttp2_mem_malloc(mem, sizeof(nghttp2_outbound_item));
  if (item == NULL) {
    return NGHTTP2_ERR_NOMEM;
  }

  nghttp2_outbound_item_init(item);

  frame = &item->frame;

  nghttp2_frame_ping_init(&frame->ping, flags, opaque_data);

  rv = nghttp2_session_add_item(session, item);

  if (rv != 0) {
    nghttp2_frame_ping_free(&frame->ping);
    nghttp2_mem_free(mem, item);
    return rv;
  }

  if (flags & NGHTTP2_FLAG_ACK) {
    ++session->obq_flood_counter_;
  }

  return 0;
}

int nghttp2_session_add_goaway(nghttp2_session *session, int32_t last_stream_id,
                               uint32_t error_code, const uint8_t *opaque_data,
                               size_t opaque_data_len, uint8_t aux_flags) {
  int rv;
  nghttp2_outbound_item *item;
  nghttp2_frame *frame;
  uint8_t *opaque_data_copy = NULL;
  nghttp2_goaway_aux_data *aux_data;
  nghttp2_mem *mem;

  mem = &session->mem;

  if (nghttp2_session_is_my_stream_id(session, last_stream_id)) {
    return NGHTTP2_ERR_INVALID_ARGUMENT;
  }

  if (opaque_data_len) {
    if (opaque_data_len + 8 > NGHTTP2_MAX_PAYLOADLEN) {
      return NGHTTP2_ERR_INVALID_ARGUMENT;
    }
    opaque_data_copy = nghttp2_mem_malloc(mem, opaque_data_len);
    if (opaque_data_copy == NULL) {
      return NGHTTP2_ERR_NOMEM;
    }
    memcpy(opaque_data_copy, opaque_data, opaque_data_len);
  }

  item = nghttp2_mem_malloc(mem, sizeof(nghttp2_outbound_item));
  if (item == NULL) {
    nghttp2_mem_free(mem, opaque_data_copy);
    return NGHTTP2_ERR_NOMEM;
  }

  nghttp2_outbound_item_init(item);

  frame = &item->frame;

  /* last_stream_id must not be increased from the value previously
     sent */
  last_stream_id = nghttp2_min(last_stream_id, session->local_last_stream_id);

  nghttp2_frame_goaway_init(&frame->goaway, last_stream_id, error_code,
                            opaque_data_copy, opaque_data_len);

  aux_data = &item->aux_data.goaway;
  aux_data->flags = aux_flags;

  rv = nghttp2_session_add_item(session, item);
  if (rv != 0) {
    nghttp2_frame_goaway_free(&frame->goaway, mem);
    nghttp2_mem_free(mem, item);
    return rv;
  }
  return 0;
}

int nghttp2_session_add_window_update(nghttp2_session *session, uint8_t flags,
                                      int32_t stream_id,
                                      int32_t window_size_increment) {
  int rv;
  nghttp2_outbound_item *item;
  nghttp2_frame *frame;
  nghttp2_mem *mem;

  mem = &session->mem;
  item = nghttp2_mem_malloc(mem, sizeof(nghttp2_outbound_item));
  if (item == NULL) {
    return NGHTTP2_ERR_NOMEM;
  }

  nghttp2_outbound_item_init(item);

  frame = &item->frame;

  nghttp2_frame_window_update_init(&frame->window_update, flags, stream_id,
                                   window_size_increment);

  rv = nghttp2_session_add_item(session, item);

  if (rv != 0) {
    nghttp2_frame_window_update_free(&frame->window_update);
    nghttp2_mem_free(mem, item);
    return rv;
  }
  return 0;
}

static void
session_append_inflight_settings(nghttp2_session *session,
                                 nghttp2_inflight_settings *settings) {
  nghttp2_inflight_settings **i;

  for (i = &session->inflight_settings_head; *i; i = &(*i)->next)
    ;

  *i = settings;
}

int nghttp2_session_add_settings(nghttp2_session *session, uint8_t flags,
                                 const nghttp2_settings_entry *iv, size_t niv) {
  nghttp2_outbound_item *item;
  nghttp2_frame *frame;
  nghttp2_settings_entry *iv_copy;
  size_t i;
  int rv;
  nghttp2_mem *mem;
  nghttp2_inflight_settings *inflight_settings = NULL;

  mem = &session->mem;

  if (flags & NGHTTP2_FLAG_ACK) {
    if (niv != 0) {
      return NGHTTP2_ERR_INVALID_ARGUMENT;
    }

    if (session->obq_flood_counter_ >= NGHTTP2_MAX_OBQ_FLOOD_ITEM) {
      return NGHTTP2_ERR_FLOODED;
    }
  }

  if (!nghttp2_iv_check(iv, niv)) {
    return NGHTTP2_ERR_INVALID_ARGUMENT;
  }

  item = nghttp2_mem_malloc(mem, sizeof(nghttp2_outbound_item));
  if (item == NULL) {
    return NGHTTP2_ERR_NOMEM;
  }

  if (niv > 0) {
    iv_copy = nghttp2_frame_iv_copy(iv, niv, mem);
    if (iv_copy == NULL) {
      nghttp2_mem_free(mem, item);
      return NGHTTP2_ERR_NOMEM;
    }
  } else {
    iv_copy = NULL;
  }

  if ((flags & NGHTTP2_FLAG_ACK) == 0) {
    rv = inflight_settings_new(&inflight_settings, iv, niv, mem);
    if (rv != 0) {
      assert(nghttp2_is_fatal(rv));
      nghttp2_mem_free(mem, iv_copy);
      nghttp2_mem_free(mem, item);
      return rv;
    }
nghttp2_session_get_hd_deflate_dynamic_table_size(nghttp2_session *session) {
  return nghttp2_hd_deflate_get_dynamic_table_size(&session->hd_deflater);
}

void nghttp2_session_set_user_data(nghttp2_session *session, void *user_data) {
  session->user_data = user_data;
}