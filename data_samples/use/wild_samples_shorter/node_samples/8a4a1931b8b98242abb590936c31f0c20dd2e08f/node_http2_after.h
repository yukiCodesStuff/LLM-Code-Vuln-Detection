  SESSION_STATE_SENDING = 0x10,
  SESSION_STATE_WRITE_IN_PROGRESS = 0x20,
  SESSION_STATE_READING_STOPPED = 0x40,
  SESSION_STATE_NGHTTP2_RECV_PAUSED = 0x80
};

typedef uint32_t(*get_setting)(nghttp2_session* session,
                               nghttp2_settings_id id);
  // Indicates whether there currently exist outgoing buffers for this stream.
  bool HasWritesOnSocketForStream(Http2Stream* stream);

  // Write data from stream_buf_ to the session
  ssize_t ConsumeHTTP2Data();

  void MemoryInfo(MemoryTracker* tracker) const override {
    tracker->TrackField("streams", streams_);
    tracker->TrackField("outstanding_pings", outstanding_pings_);
    tracker->TrackField("outstanding_settings", outstanding_settings_);
    tracker->TrackField("outgoing_buffers", outgoing_buffers_);
    tracker->TrackFieldWithSize("stream_buf", stream_buf_.len);
    tracker->TrackFieldWithSize("outgoing_storage", outgoing_storage_.size());
    tracker->TrackFieldWithSize("pending_rst_streams",
                                pending_rst_streams_.size() * sizeof(int32_t));
  }
  }

  void DecrementCurrentSessionMemory(uint64_t amount) {
    DCHECK_LE(amount, current_session_memory_);
    current_session_memory_ -= amount;
  }

  // Tell our custom memory allocator that this rcbuf is independent of
  uint32_t chunks_sent_since_last_write_ = 0;

  uv_buf_t stream_buf_ = uv_buf_init(nullptr, 0);
  // When processing input data, either stream_buf_ab_ or stream_buf_allocation_
  // will be set. stream_buf_ab_ is lazily created from stream_buf_allocation_.
  v8::Global<v8::ArrayBuffer> stream_buf_ab_;
  AllocatedBuffer stream_buf_allocation_;
  size_t stream_buf_offset_ = 0;

  size_t max_outstanding_pings_ = DEFAULT_MAX_PINGS;
  std::queue<std::unique_ptr<Http2Ping>> outstanding_pings_;


  std::vector<nghttp2_stream_write> outgoing_buffers_;
  std::vector<uint8_t> outgoing_storage_;
  size_t outgoing_length_ = 0;
  std::vector<int32_t> pending_rst_streams_;
  // Count streams that have been rejected while being opened. Exceeding a fixed
  // limit will result in the session being destroyed, as an indication of a
  // misbehaving peer. This counter is reset once new streams are being
  // Also use the invalid frame count as a measure for rejecting input frames.
  int32_t invalid_frame_count_ = 0;

  void PushOutgoingBuffer(nghttp2_stream_write&& write);
  void CopyDataIntoOutgoing(const uint8_t* src, size_t src_length);
  void ClearOutgoing(int status);

  friend class Http2Scope;