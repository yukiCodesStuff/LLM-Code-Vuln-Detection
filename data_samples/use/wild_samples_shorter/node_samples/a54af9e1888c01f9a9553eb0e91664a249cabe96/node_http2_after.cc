  Http2Session* session = static_cast<Http2Session*>(user_data);

  Debug(session, "invalid frame received, code: %d", lib_error_code);
  if (session->invalid_frame_count_++ > 1000 &&
      !IsReverted(SECURITY_REVERT_CVE_2019_9514)) {
    return 1;
  }

  // If the error is fatal or if error code is ERR_STREAM_CLOSED... emit error
  if (nghttp2_is_fatal(lib_error_code) ||
      lib_error_code == NGHTTP2_ERR_STREAM_CLOSED) {