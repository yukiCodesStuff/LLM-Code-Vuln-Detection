                              size_t* chars_written) {
  uint16_t* const dst = reinterpret_cast<uint16_t*>(buf);

  size_t max_chars = buflen / sizeof(*dst);
  if (max_chars == 0) {
    return 0;
  }