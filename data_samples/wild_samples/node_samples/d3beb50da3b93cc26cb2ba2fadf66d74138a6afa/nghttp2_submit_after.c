    if (rv != 0) {
      return rv;
    }

    if (window_size_increment > 0) {
      return nghttp2_session_add_window_update(session, 0, stream_id,
                                               window_size_increment);
    }

    return nghttp2_session_update_recv_connection_window_size(session, 0);
  } else {
    stream = nghttp2_session_get_stream(session, stream_id);

    if (stream == NULL) {
      return 0;
    }
    if (rv != 0) {
      return rv;
    }

    if (window_size_increment > 0) {
      return nghttp2_session_add_window_update(session, 0, stream_id,
                                               window_size_increment);
    }

    return nghttp2_session_update_recv_stream_window_size(session, stream, 0,
                                                          1);
  }

  return 0;
}

int nghttp2_submit_altsvc(nghttp2_session *session, uint8_t flags,
                          int32_t stream_id, const uint8_t *origin,
                          size_t origin_len, const uint8_t *field_value,
                          size_t field_value_len) {
  nghttp2_mem *mem;
  uint8_t *buf, *p;
  uint8_t *origin_copy;
  uint8_t *field_value_copy;
  nghttp2_outbound_item *item;
  nghttp2_frame *frame;
  nghttp2_ext_altsvc *altsvc;
  int rv;
  (void)flags;

  mem = &session->mem;

  if (!session->server) {
    return NGHTTP2_ERR_INVALID_STATE;
  }

  if (2 + origin_len + field_value_len > NGHTTP2_MAX_PAYLOADLEN) {
    return NGHTTP2_ERR_INVALID_ARGUMENT;
  }

  if (stream_id == 0) {
    if (origin_len == 0) {
      return NGHTTP2_ERR_INVALID_ARGUMENT;
    }
  } else if (origin_len != 0) {
    return NGHTTP2_ERR_INVALID_ARGUMENT;
  }

  buf = nghttp2_mem_malloc(mem, origin_len + field_value_len + 2);
  if (buf == NULL) {
    return NGHTTP2_ERR_NOMEM;
  }

  p = buf;

  origin_copy = p;
  if (origin_len) {
    p = nghttp2_cpymem(p, origin, origin_len);
  }
  *p++ = '\0';

  field_value_copy = p;
  if (field_value_len) {
    p = nghttp2_cpymem(p, field_value, field_value_len);
  }
  *p++ = '\0';

  item = nghttp2_mem_malloc(mem, sizeof(nghttp2_outbound_item));
  if (item == NULL) {
    rv = NGHTTP2_ERR_NOMEM;
    goto fail_item_malloc;
  }

  nghttp2_outbound_item_init(item);

  item->aux_data.ext.builtin = 1;

  altsvc = &item->ext_frame_payload.altsvc;

  frame = &item->frame;
  frame->ext.payload = altsvc;

  nghttp2_frame_altsvc_init(&frame->ext, stream_id, origin_copy, origin_len,
                            field_value_copy, field_value_len);

  rv = nghttp2_session_add_item(session, item);
  if (rv != 0) {
    nghttp2_frame_altsvc_free(&frame->ext, mem);
    nghttp2_mem_free(mem, item);

    return rv;
  }

  return 0;

fail_item_malloc:
  free(buf);

  return rv;
}

int nghttp2_submit_origin(nghttp2_session *session, uint8_t flags,
                          const nghttp2_origin_entry *ov, size_t nov) {
  nghttp2_mem *mem;
  uint8_t *p;
  nghttp2_outbound_item *item;
  nghttp2_frame *frame;
  nghttp2_ext_origin *origin;
  nghttp2_origin_entry *ov_copy;
  size_t len = 0;
  size_t i;
  int rv;
  (void)flags;

  mem = &session->mem;

  if (!session->server) {
    return NGHTTP2_ERR_INVALID_STATE;
  }

  if (nov) {
    for (i = 0; i < nov; ++i) {
      len += ov[i].origin_len;
    }