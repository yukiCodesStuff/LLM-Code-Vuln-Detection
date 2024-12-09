  uint32_t chunks_sent_since_last_write_ = 0;

  uv_buf_t stream_buf_ = uv_buf_init(nullptr, 0);
  v8::Local<v8::ArrayBuffer> stream_buf_ab_;

  size_t max_outstanding_pings_ = DEFAULT_MAX_PINGS;
  std::queue<std::unique_ptr<Http2Ping>> outstanding_pings_;
