  //            terms of MB increments (i.e. the value 1 == 1 MB)
  if (flags & (1 << IDX_OPTIONS_MAX_SESSION_MEMORY))
    set_max_session_memory(buffer[IDX_OPTIONS_MAX_SESSION_MEMORY] * 1000000);

  if (flags & (1 << IDX_OPTIONS_MAX_SETTINGS)) {
    nghttp2_option_set_max_settings(
        option,
        static_cast<size_t>(buffer[IDX_OPTIONS_MAX_SETTINGS]));
  }
}

#define GRABSETTING(entries, count, name)                                      \
  do {                                                                         \