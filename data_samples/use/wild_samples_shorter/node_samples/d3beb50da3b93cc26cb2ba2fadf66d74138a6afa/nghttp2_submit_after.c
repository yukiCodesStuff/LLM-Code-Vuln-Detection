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