  if (session->goaway_flags & NGHTTP2_GOAWAY_TERM_ON_SEND) {
    return 0;
  }

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

    if (session_is_closing(session)) {
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

        iframe->state = NGHTTP2_IB_IGN_ALL;

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

        busy = 1;

        iframe->state = NGHTTP2_IB_IGN_PAYLOAD;

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
        if (rv == NGHTTP2_ERR_IGN_PAYLOAD) {
          DEBUGF("recv: DATA not allowed stream_id=%d\n",
                 iframe->frame.hd.stream_id);
          iframe->state = NGHTTP2_IB_IGN_DATA;
          break;
        }
        if (nghttp2_is_fatal(rv)) {
          return rv;
        }

        rv = inbound_frame_handle_pad(iframe, &iframe->frame.hd);
        if (rv < 0) {
          iframe->state = NGHTTP2_IB_IGN_DATA;
          rv = nghttp2_session_terminate_session_with_reason(
              session, NGHTTP2_PROTOCOL_ERROR,
              "DATA: insufficient padding space");

          if (nghttp2_is_fatal(rv)) {
            return rv;
          }
          break;
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
          busy = 1;

          iframe->state = NGHTTP2_IB_IGN_PAYLOAD;

          rv = nghttp2_session_terminate_session_with_reason(
              session, NGHTTP2_PROTOCOL_ERROR,
              "HEADERS: insufficient padding space");
          if (nghttp2_is_fatal(rv)) {
            return rv;
          }
          break;
        }
        if (nghttp2_is_fatal(rv)) {
          return rv;
        }

        busy = 1;

        if (rv == NGHTTP2_ERR_TEMPORAL_CALLBACK_FAILURE) {
          rv = nghttp2_session_add_rst_stream(
              session, iframe->frame.hd.stream_id, NGHTTP2_INTERNAL_ERROR);
          if (nghttp2_is_fatal(rv)) {
            return rv;
          }
          iframe->state = NGHTTP2_IB_IGN_HEADER_BLOCK;
          break;
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
          busy = 1;
          iframe->state = NGHTTP2_IB_IGN_PAYLOAD;
          rv = nghttp2_session_terminate_session_with_reason(
              session, NGHTTP2_PROTOCOL_ERROR,
              "PUSH_PROMISE: insufficient padding space");
          if (nghttp2_is_fatal(rv)) {
            return rv;
          }
          break;
        }
        if (nghttp2_is_fatal(rv)) {
          return rv;
        }

        busy = 1;

        iframe->state = NGHTTP2_IB_IGN_PAYLOAD;

        break;
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
          padlen = inbound_frame_compute_pad(iframe);
          if (padlen < 0) {
            busy = 1;
            rv = nghttp2_session_terminate_session_with_reason(
                session, NGHTTP2_PROTOCOL_ERROR, "HEADERS: invalid padding");
            if (nghttp2_is_fatal(rv)) {
              return rv;
            }
            iframe->state = NGHTTP2_IB_IGN_PAYLOAD;
            break;
          }
          iframe->frame.headers.padlen = (size_t)padlen;

          pri_fieldlen = nghttp2_frame_priority_len(iframe->frame.hd.flags);

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

        session_inbound_frame_reset(session);

        break;
      case NGHTTP2_RST_STREAM:
        rv = session_process_rst_stream_frame(session);
        if (nghttp2_is_fatal(rv)) {
          return rv;
        }

        session_inbound_frame_reset(session);

        break;
      case NGHTTP2_PUSH_PROMISE:
        if (iframe->padlen == 0 &&
            (iframe->frame.hd.flags & NGHTTP2_FLAG_PADDED)) {
          padlen = inbound_frame_compute_pad(iframe);
          if (padlen < 0) {
            busy = 1;
            rv = nghttp2_session_terminate_session_with_reason(
                session, NGHTTP2_PROTOCOL_ERROR,
                "PUSH_PROMISE: invalid padding");
            if (nghttp2_is_fatal(rv)) {
              return rv;
            }
            iframe->state = NGHTTP2_IB_IGN_PAYLOAD;
            break;
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

        if (rv == NGHTTP2_ERR_TEMPORAL_CALLBACK_FAILURE) {
          rv = nghttp2_session_add_rst_stream(
              session, iframe->frame.hd.stream_id, NGHTTP2_INTERNAL_ERROR);
          if (nghttp2_is_fatal(rv)) {
            return rv;
          }
          iframe->state = NGHTTP2_IB_IGN_HEADER_BLOCK;
          break;
        }
        if (nghttp2_is_fatal(rv)) {
          return rv;
        }

        session_inbound_frame_reset(session);

        break;
      case NGHTTP2_RST_STREAM:
        rv = session_process_rst_stream_frame(session);
        if (nghttp2_is_fatal(rv)) {
          return rv;
        }

        session_inbound_frame_reset(session);

        break;
      case NGHTTP2_PUSH_PROMISE:
        if (iframe->padlen == 0 &&
            (iframe->frame.hd.flags & NGHTTP2_FLAG_PADDED)) {
          padlen = inbound_frame_compute_pad(iframe);
          if (padlen < 0) {
            busy = 1;
            rv = nghttp2_session_terminate_session_with_reason(
                session, NGHTTP2_PROTOCOL_ERROR,
                "PUSH_PROMISE: invalid padding");
            if (nghttp2_is_fatal(rv)) {
              return rv;
            }
            iframe->state = NGHTTP2_IB_IGN_PAYLOAD;
            break;
          }
        if (nghttp2_is_fatal(rv)) {
          return rv;
        }

        session_inbound_frame_reset(session);

        break;
      case NGHTTP2_PUSH_PROMISE:
        if (iframe->padlen == 0 &&
            (iframe->frame.hd.flags & NGHTTP2_FLAG_PADDED)) {
          padlen = inbound_frame_compute_pad(iframe);
          if (padlen < 0) {
            busy = 1;
            rv = nghttp2_session_terminate_session_with_reason(
                session, NGHTTP2_PROTOCOL_ERROR,
                "PUSH_PROMISE: invalid padding");
            if (nghttp2_is_fatal(rv)) {
              return rv;
            }
            iframe->state = NGHTTP2_IB_IGN_PAYLOAD;
            break;
          }
        if (nghttp2_is_fatal(rv)) {
          return rv;
        }

        busy = 1;

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
        if (nghttp2_is_fatal(rv)) {
          return rv;
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
      trail_padlen = nghttp2_frame_trail_padlen(&iframe->frame, iframe->padlen);

      final = (iframe->frame.hd.flags & NGHTTP2_FLAG_END_HEADERS) &&
              iframe->payloadleft - (size_t)data_readlen == trail_padlen;

      if (data_readlen > 0 || (data_readlen == 0 && final)) {
        size_t hd_proclen = 0;

        DEBUGF("recv: block final=%d\n", final);

        rv =
            inflate_header_block(session, &iframe->frame, &hd_proclen,
                                 (uint8_t *)in, (size_t)data_readlen, final,
                                 iframe->state == NGHTTP2_IB_READ_HEADER_BLOCK);

        if (nghttp2_is_fatal(rv)) {
          return rv;
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
          busy = 1;
          iframe->state = NGHTTP2_IB_IGN_HEADER_BLOCK;
          break;
        }

        in += readlen;
        iframe->payloadleft -= readlen;

        if (rv == NGHTTP2_ERR_HEADER_COMP) {
          /* GOAWAY is already issued */
          if (iframe->payloadleft == 0) {
            session_inbound_frame_reset(session);
          } else {
            busy = 1;
            iframe->state = NGHTTP2_IB_IGN_PAYLOAD;
          }
          break;
        }
      } else {
        in += readlen;
        iframe->payloadleft -= readlen;
      }

      if (iframe->payloadleft) {
        break;
      }

      if ((iframe->frame.hd.flags & NGHTTP2_FLAG_END_HEADERS) == 0) {

        inbound_frame_set_mark(iframe, NGHTTP2_FRAME_HDLEN);

        iframe->padlen = 0;

        if (iframe->state == NGHTTP2_IB_READ_HEADER_BLOCK) {
          iframe->state = NGHTTP2_IB_EXPECT_CONTINUATION;
        } else {
          iframe->state = NGHTTP2_IB_IGN_CONTINUATION;
        }
      } else {
        if (iframe->state == NGHTTP2_IB_READ_HEADER_BLOCK) {
          rv = session_after_header_block_received(session);
          if (nghttp2_is_fatal(rv)) {
            return rv;
          }
        }
        session_inbound_frame_reset(session);
      }
      break;
    }
    case NGHTTP2_IB_IGN_PAYLOAD:
      DEBUGF("recv: [IB_IGN_PAYLOAD]\n");

      readlen = inbound_frame_payload_readlen(iframe, in, last);
      iframe->payloadleft -= readlen;
      in += readlen;

      DEBUGF("recv: readlen=%zu, payloadleft=%zu\n", readlen,
             iframe->payloadleft);

      if (iframe->payloadleft) {
        break;
      }

      switch (iframe->frame.hd.type) {
      case NGHTTP2_HEADERS:
      case NGHTTP2_PUSH_PROMISE:
      case NGHTTP2_CONTINUATION:
        /* Mark inflater bad so that we won't perform further decoding */
        session->hd_inflater.ctx.bad = 1;
        break;
      default:
        break;
      }

      session_inbound_frame_reset(session);

      break;
    case NGHTTP2_IB_FRAME_SIZE_ERROR:
      DEBUGF("recv: [IB_FRAME_SIZE_ERROR]\n");

      rv = session_handle_frame_size_error(session);
      if (nghttp2_is_fatal(rv)) {
        return rv;
      }

      busy = 1;

      iframe->state = NGHTTP2_IB_IGN_PAYLOAD;

      break;
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
#endif /* DEBUGBUILD */

      readlen = inbound_frame_buf_read(iframe, in, last);
      in += readlen;

      if (nghttp2_buf_mark_avail(&iframe->sbuf)) {
        return in - first;
      }

      nghttp2_frame_unpack_frame_hd(&cont_hd, iframe->sbuf.pos);
      iframe->payloadleft = cont_hd.length;

      DEBUGF("recv: payloadlen=%zu, type=%u, flags=0x%02x, stream_id=%d\n",
             cont_hd.length, cont_hd.type, cont_hd.flags, cont_hd.stream_id);

      if (cont_hd.type != NGHTTP2_CONTINUATION ||
          cont_hd.stream_id != iframe->frame.hd.stream_id) {
        DEBUGF("recv: expected stream_id=%d, type=%d, but got stream_id=%d, "
               "type=%u\n",
               iframe->frame.hd.stream_id, NGHTTP2_CONTINUATION,
               cont_hd.stream_id, cont_hd.type);
        rv = nghttp2_session_terminate_session_with_reason(
            session, NGHTTP2_PROTOCOL_ERROR,
            "unexpected non-CONTINUATION frame or stream_id is invalid");
        if (nghttp2_is_fatal(rv)) {
          return rv;
        }

        busy = 1;

        iframe->state = NGHTTP2_IB_IGN_PAYLOAD;

        break;
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

      /* Pad Length field is consumed immediately */
      rv =
          nghttp2_session_consume(session, iframe->frame.hd.stream_id, readlen);

      if (nghttp2_is_fatal(rv)) {
        return rv;
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
      }

      busy = 1;

      padlen = inbound_frame_compute_pad(iframe);
      if (padlen < 0) {
        rv = nghttp2_session_terminate_session_with_reason(
            session, NGHTTP2_PROTOCOL_ERROR, "DATA: invalid padding");
        if (nghttp2_is_fatal(rv)) {
          return rv;
        }
        iframe->state = NGHTTP2_IB_IGN_DATA;
        break;
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

        if (session->opt_flags & NGHTTP2_OPTMASK_NO_AUTO_WINDOW_UPDATE) {

          /* Ignored DATA is considered as "consumed" immediately. */
          rv = session_update_connection_consumed_size(session, readlen);

          if (nghttp2_is_fatal(rv)) {
            return rv;
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
    }
    datamax = (size_t)payloadlen;
  }

  /* Current max DATA length is less then buffer chunk size */
  assert(nghttp2_buf_avail(buf) >= datamax);

  data_flags = NGHTTP2_DATA_FLAG_NONE;
  payloadlen = aux_data->data_prd.read_callback(
      session, frame->hd.stream_id, buf->pos, datamax, &data_flags,
      &aux_data->data_prd.source, session->user_data);

  if (payloadlen == NGHTTP2_ERR_DEFERRED ||
      payloadlen == NGHTTP2_ERR_TEMPORAL_CALLBACK_FAILURE ||
      payloadlen == NGHTTP2_ERR_PAUSE) {
    DEBUGF("send: DATA postponed due to %s\n",
           nghttp2_strerror((int)payloadlen));

    return (int)payloadlen;
  }

  if (payloadlen < 0 || datamax < (size_t)payloadlen) {
    /* This is the error code when callback is failed. */
    return NGHTTP2_ERR_CALLBACK_FAILURE;
  }

  buf->last = buf->pos + payloadlen;
  buf->pos -= NGHTTP2_FRAME_HDLEN;

  /* Clear flags, because this may contain previous flags of previous
     DATA */
  frame->hd.flags = NGHTTP2_FLAG_NONE;

  if (data_flags & NGHTTP2_DATA_FLAG_EOF) {
    aux_data->eof = 1;
    /* If NGHTTP2_DATA_FLAG_NO_END_STREAM is set, don't set
       NGHTTP2_FLAG_END_STREAM */
    if ((aux_data->flags & NGHTTP2_FLAG_END_STREAM) &&
        (data_flags & NGHTTP2_DATA_FLAG_NO_END_STREAM) == 0) {
      frame->hd.flags |= NGHTTP2_FLAG_END_STREAM;
    }
  }

  if (data_flags & NGHTTP2_DATA_FLAG_NO_COPY) {
    if (session->callbacks.send_data_callback == NULL) {
      DEBUGF("NGHTTP2_DATA_FLAG_NO_COPY requires send_data_callback set\n");

      return NGHTTP2_ERR_CALLBACK_FAILURE;
    }
    aux_data->no_copy = 1;
  }

  frame->hd.length = (size_t)payloadlen;
  frame->data.padlen = 0;

  max_payloadlen = nghttp2_min(datamax, frame->hd.length + NGHTTP2_MAX_PADLEN);

  padded_payloadlen =
      session_call_select_padding(session, frame, max_payloadlen);

  if (nghttp2_is_fatal((int)padded_payloadlen)) {
    return (int)padded_payloadlen;
  }

  frame->data.padlen = (size_t)(padded_payloadlen - payloadlen);

  nghttp2_frame_pack_frame_hd(buf->pos, &frame->hd);

  rv = nghttp2_frame_add_pad(bufs, &frame->hd, frame->data.padlen,
                             aux_data->no_copy);
  if (rv != 0) {
    return rv;
  }

  reschedule_stream(stream);

  if (frame->hd.length == 0 && (data_flags & NGHTTP2_DATA_FLAG_EOF) &&
      (data_flags & NGHTTP2_DATA_FLAG_NO_END_STREAM)) {
    /* DATA payload length is 0, and DATA frame does not bear
       END_STREAM.  In this case, there is no point to send 0 length
       DATA frame. */
    return NGHTTP2_ERR_CANCEL;
  }

  return 0;
}

void *nghttp2_session_get_stream_user_data(nghttp2_session *session,
                                           int32_t stream_id) {
  nghttp2_stream *stream;
  stream = nghttp2_session_get_stream(session, stream_id);
  if (stream) {
    return stream->stream_user_data;
  } else {
    return NULL;
  }
}

int nghttp2_session_set_stream_user_data(nghttp2_session *session,
                                         int32_t stream_id,
                                         void *stream_user_data) {
  nghttp2_stream *stream;
  stream = nghttp2_session_get_stream(session, stream_id);
  if (!stream) {
    return NGHTTP2_ERR_INVALID_ARGUMENT;
  }
  stream->stream_user_data = stream_user_data;
  return 0;
}

int nghttp2_session_resume_data(nghttp2_session *session, int32_t stream_id) {
  int rv;
  nghttp2_stream *stream;
  stream = nghttp2_session_get_stream(session, stream_id);
  if (stream == NULL || !nghttp2_stream_check_deferred_item(stream)) {
    return NGHTTP2_ERR_INVALID_ARGUMENT;
  }

  rv = nghttp2_stream_resume_deferred_item(stream,
                                           NGHTTP2_STREAM_FLAG_DEFERRED_USER);

  if (nghttp2_is_fatal(rv)) {
    return rv;
  }

  return 0;
}

size_t nghttp2_session_get_outbound_queue_size(nghttp2_session *session) {
  return nghttp2_outbound_queue_size(&session->ob_urgent) +
         nghttp2_outbound_queue_size(&session->ob_reg) +
         nghttp2_outbound_queue_size(&session->ob_syn);
  /* TODO account for item attached to stream */
}

int32_t
nghttp2_session_get_stream_effective_recv_data_length(nghttp2_session *session,
                                                      int32_t stream_id) {
  nghttp2_stream *stream;
  stream = nghttp2_session_get_stream(session, stream_id);
  if (stream == NULL) {
    return -1;
  }
  return stream->recv_window_size < 0 ? 0 : stream->recv_window_size;
}

int32_t
nghttp2_session_get_stream_effective_local_window_size(nghttp2_session *session,
                                                       int32_t stream_id) {
  nghttp2_stream *stream;
  stream = nghttp2_session_get_stream(session, stream_id);
  if (stream == NULL) {
    return -1;
  }
  return stream->local_window_size;
}

int32_t nghttp2_session_get_stream_local_window_size(nghttp2_session *session,
                                                     int32_t stream_id) {
  nghttp2_stream *stream;
  int32_t size;
  stream = nghttp2_session_get_stream(session, stream_id);
  if (stream == NULL) {
    return -1;
  }

  size = stream->local_window_size - stream->recv_window_size;

  /* size could be negative if local endpoint reduced
     SETTINGS_INITIAL_WINDOW_SIZE */
  if (size < 0) {
    return 0;
  }

  return size;
}

int32_t
nghttp2_session_get_effective_recv_data_length(nghttp2_session *session) {
  return session->recv_window_size < 0 ? 0 : session->recv_window_size;
}

int32_t
nghttp2_session_get_effective_local_window_size(nghttp2_session *session) {
  return session->local_window_size;
}

int32_t nghttp2_session_get_local_window_size(nghttp2_session *session) {
  return session->local_window_size - session->recv_window_size;
}

int32_t nghttp2_session_get_stream_remote_window_size(nghttp2_session *session,
                                                      int32_t stream_id) {
  nghttp2_stream *stream;

  stream = nghttp2_session_get_stream(session, stream_id);
  if (stream == NULL) {
    return -1;
  }

  /* stream->remote_window_size can be negative when
     SETTINGS_INITIAL_WINDOW_SIZE is changed. */
  return nghttp2_max(0, stream->remote_window_size);
}

int32_t nghttp2_session_get_remote_window_size(nghttp2_session *session) {
  return session->remote_window_size;
}

uint32_t nghttp2_session_get_remote_settings(nghttp2_session *session,
                                             nghttp2_settings_id id) {
  switch (id) {
  case NGHTTP2_SETTINGS_HEADER_TABLE_SIZE:
    return session->remote_settings.header_table_size;
  case NGHTTP2_SETTINGS_ENABLE_PUSH:
    return session->remote_settings.enable_push;
  case NGHTTP2_SETTINGS_MAX_CONCURRENT_STREAMS:
    return session->remote_settings.max_concurrent_streams;
  case NGHTTP2_SETTINGS_INITIAL_WINDOW_SIZE:
    return session->remote_settings.initial_window_size;
  case NGHTTP2_SETTINGS_MAX_FRAME_SIZE:
    return session->remote_settings.max_frame_size;
  case NGHTTP2_SETTINGS_MAX_HEADER_LIST_SIZE:
    return session->remote_settings.max_header_list_size;
  }

  assert(0);
  abort(); /* if NDEBUG is set */
}

uint32_t nghttp2_session_get_local_settings(nghttp2_session *session,
                                            nghttp2_settings_id id) {
  switch (id) {
  case NGHTTP2_SETTINGS_HEADER_TABLE_SIZE:
    return session->local_settings.header_table_size;
  case NGHTTP2_SETTINGS_ENABLE_PUSH:
    return session->local_settings.enable_push;
  case NGHTTP2_SETTINGS_MAX_CONCURRENT_STREAMS:
    return session->local_settings.max_concurrent_streams;
  case NGHTTP2_SETTINGS_INITIAL_WINDOW_SIZE:
    return session->local_settings.initial_window_size;
  case NGHTTP2_SETTINGS_MAX_FRAME_SIZE:
    return session->local_settings.max_frame_size;
  case NGHTTP2_SETTINGS_MAX_HEADER_LIST_SIZE:
    return session->local_settings.max_header_list_size;
  }

  assert(0);
  abort(); /* if NDEBUG is set */
}

static int nghttp2_session_upgrade_internal(nghttp2_session *session,
                                            const uint8_t *settings_payload,
                                            size_t settings_payloadlen,
                                            void *stream_user_data) {
  nghttp2_stream *stream;
  nghttp2_frame frame;
  nghttp2_settings_entry *iv;
  size_t niv;
  int rv;
  nghttp2_priority_spec pri_spec;
  nghttp2_mem *mem;

  mem = &session->mem;

  if ((!session->server && session->next_stream_id != 1) ||
      (session->server && session->last_recv_stream_id >= 1)) {
    return NGHTTP2_ERR_PROTO;
  }
  if (settings_payloadlen % NGHTTP2_FRAME_SETTINGS_ENTRY_LENGTH) {
    return NGHTTP2_ERR_INVALID_ARGUMENT;
  }
  rv = nghttp2_frame_unpack_settings_payload2(&iv, &niv, settings_payload,
                                              settings_payloadlen, mem);
  if (rv != 0) {
    return rv;
  }

  if (session->server) {
    nghttp2_frame_hd_init(&frame.hd, settings_payloadlen, NGHTTP2_SETTINGS,
                          NGHTTP2_FLAG_NONE, 0);
    frame.settings.iv = iv;
    frame.settings.niv = niv;
    rv = nghttp2_session_on_settings_received(session, &frame, 1 /* No ACK */);
  } else {
    rv = nghttp2_submit_settings(session, NGHTTP2_FLAG_NONE, iv, niv);
  }
  nghttp2_mem_free(mem, iv);
  if (rv != 0) {
    return rv;
  }

  nghttp2_priority_spec_default_init(&pri_spec);

  stream = nghttp2_session_open_stream(
      session, 1, NGHTTP2_STREAM_FLAG_NONE, &pri_spec, NGHTTP2_STREAM_OPENING,
      session->server ? NULL : stream_user_data);
  if (stream == NULL) {
    return NGHTTP2_ERR_NOMEM;
  }

  /* We don't call nghttp2_session_adjust_closed_stream(), since this
     should be the first stream open. */

  if (session->server) {
    nghttp2_stream_shutdown(stream, NGHTTP2_SHUT_RD);
    session->last_recv_stream_id = 1;
    session->last_proc_stream_id = 1;
  } else {
    nghttp2_stream_shutdown(stream, NGHTTP2_SHUT_WR);
    session->last_sent_stream_id = 1;
    session->next_stream_id += 2;
  }
  return 0;
}

int nghttp2_session_upgrade(nghttp2_session *session,
                            const uint8_t *settings_payload,
                            size_t settings_payloadlen,
                            void *stream_user_data) {
  int rv;
  nghttp2_stream *stream;

  rv = nghttp2_session_upgrade_internal(session, settings_payload,
                                        settings_payloadlen, stream_user_data);
  if (rv != 0) {
    return rv;
  }

  stream = nghttp2_session_get_stream(session, 1);
  assert(stream);

  /* We have no information about request header fields when Upgrade
     was happened.  So we don't know the request method here.  If
     request method is HEAD, we have a trouble because we may have
     nonzero content-length header field in response headers, and we
     will going to check it against the actual DATA frames, but we may
     get mismatch because HEAD response body must be empty.  Because
     of this reason, nghttp2_session_upgrade() was deprecated in favor
     of nghttp2_session_upgrade2(), which has |head_request| parameter
     to indicate that request method is HEAD or not. */
  stream->http_flags |= NGHTTP2_HTTP_FLAG_METH_UPGRADE_WORKAROUND;
  return 0;
}

int nghttp2_session_upgrade2(nghttp2_session *session,
                             const uint8_t *settings_payload,
                             size_t settings_payloadlen, int head_request,
                             void *stream_user_data) {
  int rv;
  nghttp2_stream *stream;

  rv = nghttp2_session_upgrade_internal(session, settings_payload,
                                        settings_payloadlen, stream_user_data);
  if (rv != 0) {
    return rv;
  }

  stream = nghttp2_session_get_stream(session, 1);
  assert(stream);

  if (head_request) {
    stream->http_flags |= NGHTTP2_HTTP_FLAG_METH_HEAD;
  }

  return 0;
}

int nghttp2_session_get_stream_local_close(nghttp2_session *session,
                                           int32_t stream_id) {
  nghttp2_stream *stream;

  stream = nghttp2_session_get_stream(session, stream_id);

  if (!stream) {
    return -1;
  }

  return (stream->shut_flags & NGHTTP2_SHUT_WR) != 0;
}

int nghttp2_session_get_stream_remote_close(nghttp2_session *session,
                                            int32_t stream_id) {
  nghttp2_stream *stream;

  stream = nghttp2_session_get_stream(session, stream_id);

  if (!stream) {
    return -1;
  }

  return (stream->shut_flags & NGHTTP2_SHUT_RD) != 0;
}

int nghttp2_session_consume(nghttp2_session *session, int32_t stream_id,
                            size_t size) {
  int rv;
  nghttp2_stream *stream;

  if (stream_id == 0) {
    return NGHTTP2_ERR_INVALID_ARGUMENT;
  }

  if (!(session->opt_flags & NGHTTP2_OPTMASK_NO_AUTO_WINDOW_UPDATE)) {
    return NGHTTP2_ERR_INVALID_STATE;
  }

  rv = session_update_connection_consumed_size(session, size);

  if (nghttp2_is_fatal(rv)) {
    return rv;
  }

  stream = nghttp2_session_get_stream(session, stream_id);

  if (!stream) {
    return 0;
  }

  rv = session_update_stream_consumed_size(session, stream, size);

  if (nghttp2_is_fatal(rv)) {
    return rv;
  }

  return 0;
}

int nghttp2_session_consume_connection(nghttp2_session *session, size_t size) {
  int rv;

  if (!(session->opt_flags & NGHTTP2_OPTMASK_NO_AUTO_WINDOW_UPDATE)) {
    return NGHTTP2_ERR_INVALID_STATE;
  }

  rv = session_update_connection_consumed_size(session, size);

  if (nghttp2_is_fatal(rv)) {
    return rv;
  }

  return 0;
}

int nghttp2_session_consume_stream(nghttp2_session *session, int32_t stream_id,
                                   size_t size) {
  int rv;
  nghttp2_stream *stream;

  if (stream_id == 0) {
    return NGHTTP2_ERR_INVALID_ARGUMENT;
  }

  if (!(session->opt_flags & NGHTTP2_OPTMASK_NO_AUTO_WINDOW_UPDATE)) {
    return NGHTTP2_ERR_INVALID_STATE;
  }

  stream = nghttp2_session_get_stream(session, stream_id);

  if (!stream) {
    return 0;
  }

  rv = session_update_stream_consumed_size(session, stream, size);

  if (nghttp2_is_fatal(rv)) {
    return rv;
  }

  return 0;
}

int nghttp2_session_set_next_stream_id(nghttp2_session *session,
                                       int32_t next_stream_id) {
  if (next_stream_id <= 0 ||
      session->next_stream_id > (uint32_t)next_stream_id) {
    return NGHTTP2_ERR_INVALID_ARGUMENT;
  }

  if (session->server) {
    if (next_stream_id % 2) {
      return NGHTTP2_ERR_INVALID_ARGUMENT;
    }
  } else if (next_stream_id % 2 == 0) {
    return NGHTTP2_ERR_INVALID_ARGUMENT;
  }

  session->next_stream_id = (uint32_t)next_stream_id;
  return 0;
}

uint32_t nghttp2_session_get_next_stream_id(nghttp2_session *session) {
  return session->next_stream_id;
}

int32_t nghttp2_session_get_last_proc_stream_id(nghttp2_session *session) {
  return session->last_proc_stream_id;
}

nghttp2_stream *nghttp2_session_find_stream(nghttp2_session *session,
                                            int32_t stream_id) {
  if (stream_id == 0) {
    return &session->root;
  }

  return nghttp2_session_get_stream_raw(session, stream_id);
}

nghttp2_stream *nghttp2_session_get_root_stream(nghttp2_session *session) {
  return &session->root;
}

int nghttp2_session_check_server_session(nghttp2_session *session) {
  return session->server;
}

int nghttp2_session_change_stream_priority(
    nghttp2_session *session, int32_t stream_id,
    const nghttp2_priority_spec *pri_spec) {
  int rv;
  nghttp2_stream *stream;
  nghttp2_priority_spec pri_spec_copy;

  if (stream_id == 0 || stream_id == pri_spec->stream_id) {
    return NGHTTP2_ERR_INVALID_ARGUMENT;
  }

  stream = nghttp2_session_get_stream_raw(session, stream_id);
  if (!stream) {
    return NGHTTP2_ERR_INVALID_ARGUMENT;
  }

  pri_spec_copy = *pri_spec;
  nghttp2_priority_spec_normalize_weight(&pri_spec_copy);

  rv = nghttp2_session_reprioritize_stream(session, stream, &pri_spec_copy);

  if (nghttp2_is_fatal(rv)) {
    return rv;
  }

  /* We don't intentionally call nghttp2_session_adjust_idle_stream()
     so that idle stream created by this function, and existing ones
     are kept for application.  We will adjust number of idle stream
     in nghttp2_session_mem_send or nghttp2_session_mem_recv is
     called. */
  return 0;
}

int nghttp2_session_create_idle_stream(nghttp2_session *session,
                                       int32_t stream_id,
                                       const nghttp2_priority_spec *pri_spec) {
  nghttp2_stream *stream;
  nghttp2_priority_spec pri_spec_copy;

  if (stream_id == 0 || stream_id == pri_spec->stream_id ||
      !session_detect_idle_stream(session, stream_id)) {
    return NGHTTP2_ERR_INVALID_ARGUMENT;
  }

  stream = nghttp2_session_get_stream_raw(session, stream_id);
  if (stream) {
    return NGHTTP2_ERR_INVALID_ARGUMENT;
  }

  pri_spec_copy = *pri_spec;
  nghttp2_priority_spec_normalize_weight(&pri_spec_copy);

  stream =
      nghttp2_session_open_stream(session, stream_id, NGHTTP2_STREAM_FLAG_NONE,
                                  &pri_spec_copy, NGHTTP2_STREAM_IDLE, NULL);
  if (!stream) {
    return NGHTTP2_ERR_NOMEM;
  }

  /* We don't intentionally call nghttp2_session_adjust_idle_stream()
     so that idle stream created by this function, and existing ones
     are kept for application.  We will adjust number of idle stream
     in nghttp2_session_mem_send or nghttp2_session_mem_recv is
     called. */
  return 0;
}

size_t
nghttp2_session_get_hd_inflate_dynamic_table_size(nghttp2_session *session) {
  return nghttp2_hd_inflate_get_dynamic_table_size(&session->hd_inflater);
}

size_t
nghttp2_session_get_hd_deflate_dynamic_table_size(nghttp2_session *session) {
  return nghttp2_hd_deflate_get_dynamic_table_size(&session->hd_deflater);
}
        if (nghttp2_is_fatal(rv)) {
          return rv;
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

      busy = 1;

      iframe->state = NGHTTP2_IB_IGN_PAYLOAD;

      break;
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

        busy = 1;

        iframe->state = NGHTTP2_IB_IGN_PAYLOAD;

        break;
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

      /* Pad Length field is consumed immediately */
      rv =
          nghttp2_session_consume(session, iframe->frame.hd.stream_id, readlen);

      if (nghttp2_is_fatal(rv)) {
        return rv;
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

      /* Pad Length field is consumed immediately */
      rv =
          nghttp2_session_consume(session, iframe->frame.hd.stream_id, readlen);

      if (nghttp2_is_fatal(rv)) {
        return rv;
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
        iframe->state = NGHTTP2_IB_IGN_DATA;
        break;
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

        if (session->opt_flags & NGHTTP2_OPTMASK_NO_AUTO_WINDOW_UPDATE) {

          /* Ignored DATA is considered as "consumed" immediately. */
          rv = session_update_connection_consumed_size(session, readlen);

          if (nghttp2_is_fatal(rv)) {
            return rv;
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

        if (session->opt_flags & NGHTTP2_OPTMASK_NO_AUTO_WINDOW_UPDATE) {

          /* Ignored DATA is considered as "consumed" immediately. */
          rv = session_update_connection_consumed_size(session, readlen);

          if (nghttp2_is_fatal(rv)) {
            return rv;
          }
          if (nghttp2_is_fatal(rv)) {
            return rv;
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