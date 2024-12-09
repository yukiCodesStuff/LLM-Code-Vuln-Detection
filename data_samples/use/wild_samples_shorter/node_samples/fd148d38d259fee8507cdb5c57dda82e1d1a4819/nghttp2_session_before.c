  (*session_ptr)->remote_settings.max_concurrent_streams = 100;

  (*session_ptr)->max_send_header_block_length = NGHTTP2_MAX_HEADERSLEN;

  if (option) {
    if ((option->opt_set_mask & NGHTTP2_OPT_NO_AUTO_WINDOW_UPDATE) &&
        option->no_auto_window_update) {
        option->no_closed_streams) {
      (*session_ptr)->opt_flags |= NGHTTP2_OPTMASK_NO_CLOSED_STREAMS;
    }
  }

  rv = nghttp2_hd_deflate_init2(&(*session_ptr)->hd_deflater,
                                max_deflate_dynamic_table_size, mem);
  mem = &session->mem;

  if ((flags & NGHTTP2_FLAG_ACK) &&
      session->obq_flood_counter_ >= NGHTTP2_MAX_OBQ_FLOOD_ITEM) {
    return NGHTTP2_ERR_FLOODED;
  }

  item = nghttp2_mem_malloc(mem, sizeof(nghttp2_outbound_item));
      return NGHTTP2_ERR_INVALID_ARGUMENT;
    }

    if (session->obq_flood_counter_ >= NGHTTP2_MAX_OBQ_FLOOD_ITEM) {
      return NGHTTP2_ERR_FLOODED;
    }
  }
