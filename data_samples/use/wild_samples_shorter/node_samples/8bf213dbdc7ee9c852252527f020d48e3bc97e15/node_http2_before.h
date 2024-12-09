  Statistics statistics_ = {};

 private:
  Http2Session* session_;                       // The Parent HTTP/2 Session
  int32_t id_;                                  // The Stream Identifier
  int32_t code_ = NGHTTP2_NO_ERROR;             // The RST_STREAM code (if any)
  int flags_ = NGHTTP2_STREAM_FLAG_NONE;        // Internal state flags

  uint32_t max_header_pairs_ = DEFAULT_MAX_HEADER_LIST_PAIRS;