    if (rv != 0) {
      return rv;
    }
  } else {
    stream = nghttp2_session_get_stream(session, stream_id);

    if (stream == NULL) {
    if (rv != 0) {
      return rv;
    }
  }

  if (window_size_increment > 0) {
    return nghttp2_session_add_window_update(session, 0, stream_id,
                                             window_size_increment);
  }

  return 0;
}