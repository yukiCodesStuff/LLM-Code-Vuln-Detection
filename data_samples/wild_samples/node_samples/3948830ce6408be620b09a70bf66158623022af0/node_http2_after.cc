  if (flags & (1 << IDX_OPTIONS_PADDING_STRATEGY)) {
    PaddingStrategy strategy =
        static_cast<PaddingStrategy>(
            buffer.GetValue(IDX_OPTIONS_PADDING_STRATEGY));
    set_padding_strategy(strategy);
  }

  // The max header list pairs option controls the maximum number of
  // header pairs the session may accept. This is a hard limit.. that is,
  // if the remote peer sends more than this amount, the stream will be
  // automatically closed with an RST_STREAM.
  if (flags & (1 << IDX_OPTIONS_MAX_HEADER_LIST_PAIRS))
    set_max_header_pairs(buffer[IDX_OPTIONS_MAX_HEADER_LIST_PAIRS]);

  // The HTTP2 specification places no limits on the number of HTTP2
  // PING frames that can be sent. In order to prevent PINGS from being
  // abused as an attack vector, however, we place a strict upper limit
  // on the number of unacknowledged PINGS that can be sent at any given
  // time.
  if (flags & (1 << IDX_OPTIONS_MAX_OUTSTANDING_PINGS))
    set_max_outstanding_pings(buffer[IDX_OPTIONS_MAX_OUTSTANDING_PINGS]);

  // The HTTP2 specification places no limits on the number of HTTP2
  // SETTINGS frames that can be sent. In order to prevent PINGS from being
  // abused as an attack vector, however, we place a strict upper limit
  // on the number of unacknowledged SETTINGS that can be sent at any given
  // time.
  if (flags & (1 << IDX_OPTIONS_MAX_OUTSTANDING_SETTINGS))
    set_max_outstanding_settings(buffer[IDX_OPTIONS_MAX_OUTSTANDING_SETTINGS]);

  // The HTTP2 specification places no limits on the amount of memory
  // that a session can consume. In order to prevent abuse, we place a
  // cap on the amount of memory a session can consume at any given time.
  // this is a credit based system. Existing streams may cause the limit
  // to be temporarily exceeded but once over the limit, new streams cannot
  // created.
  // Important: The maxSessionMemory option in javascript is expressed in
  //            terms of MB increments (i.e. the value 1 == 1 MB)
  if (flags & (1 << IDX_OPTIONS_MAX_SESSION_MEMORY))
    set_max_session_memory(buffer[IDX_OPTIONS_MAX_SESSION_MEMORY] * 1000000);

  if (flags & (1 << IDX_OPTIONS_MAX_SETTINGS)) {
    nghttp2_option_set_max_settings(
        option,
        static_cast<size_t>(buffer[IDX_OPTIONS_MAX_SETTINGS]));
  }