  struct Statistics {
    uint64_t start_time;
    uint64_t end_time;
    uint64_t ping_rtt;
    uint64_t data_sent;
    uint64_t data_received;
    uint32_t frame_count;
    uint32_t frame_sent;
    int32_t stream_count;
    size_t max_concurrent_streams;
    double stream_average_duration;
  };

  Statistics statistics_ = {};

 private:
  // Frame Padding Strategies
  ssize_t OnDWordAlignedPadding(size_t frameLength,
                                size_t maxPayloadLen);
  ssize_t OnMaxFrameSizePadding(size_t frameLength,
                                size_t maxPayloadLen);
  ssize_t OnCallbackPadding(size_t frameLength,
                            size_t maxPayloadLen);

  // Frame Handler
  int HandleDataFrame(const nghttp2_frame* frame);
  void HandleGoawayFrame(const nghttp2_frame* frame);
  void HandleHeadersFrame(const nghttp2_frame* frame);
  void HandlePriorityFrame(const nghttp2_frame* frame);
  void HandleSettingsFrame(const nghttp2_frame* frame);
  void HandlePingFrame(const nghttp2_frame* frame);
  void HandleAltSvcFrame(const nghttp2_frame* frame);
  void HandleOriginFrame(const nghttp2_frame* frame);

  // nghttp2 callbacks
  static int OnBeginHeadersCallback(
      nghttp2_session* session,
      const nghttp2_frame* frame,
      void* user_data);
  static int OnHeaderCallback(
      nghttp2_session* session,
      const nghttp2_frame* frame,
      nghttp2_rcbuf* name,
      nghttp2_rcbuf* value,
      uint8_t flags,
      void* user_data);
  static int OnFrameReceive(
      nghttp2_session* session,
      const nghttp2_frame* frame,
      void* user_data);
  static int OnFrameNotSent(
      nghttp2_session* session,
      const nghttp2_frame* frame,
      int error_code,
      void* user_data);
  static int OnFrameSent(
      nghttp2_session* session,
      const nghttp2_frame* frame,
      void* user_data);
  static int OnStreamClose(
      nghttp2_session* session,
      int32_t id,
      uint32_t code,
      void* user_data);
  static int OnInvalidHeader(
      nghttp2_session* session,
      const nghttp2_frame* frame,
      nghttp2_rcbuf* name,
      nghttp2_rcbuf* value,
      uint8_t flags,
      void* user_data);
  static int OnDataChunkReceived(
      nghttp2_session* session,
      uint8_t flags,
      int32_t id,
      const uint8_t* data,
      size_t len,
      void* user_data);
  static ssize_t OnSelectPadding(
      nghttp2_session* session,
      const nghttp2_frame* frame,
      size_t maxPayloadLen,
      void* user_data);
  static int OnNghttpError(
      nghttp2_session* session,
      const char* message,
      size_t len,
      void* user_data);
  static int OnSendData(
      nghttp2_session* session,
      nghttp2_frame* frame,
      const uint8_t* framehd,
      size_t length,
      nghttp2_data_source* source,
      void* user_data);
  static int OnInvalidFrame(
      nghttp2_session* session,
      const nghttp2_frame* frame,
      int lib_error_code,
      void* user_data);

  struct Callbacks {
    inline explicit Callbacks(bool kHasGetPaddingCallback);
    inline ~Callbacks();

    nghttp2_session_callbacks* callbacks;
  };

  /* Use callback_struct_saved[kHasGetPaddingCallback ? 1 : 0] */
  static const Callbacks callback_struct_saved[2];

  // The underlying nghttp2_session handle
  nghttp2_session* session_;

  // JS-accessible numeric fields, as indexed by SessionUint8Fields.
  uint8_t js_fields_[kSessionUint8FieldCount] = {};

  // The session type: client or server
  nghttp2_session_type session_type_;

  // The maximum number of header pairs permitted for streams on this session
  uint32_t max_header_pairs_ = DEFAULT_MAX_HEADER_LIST_PAIRS;

  // The maximum amount of memory allocated for this session
  uint64_t max_session_memory_ = DEFAULT_MAX_SESSION_MEMORY;
  uint64_t current_session_memory_ = 0;
  // The amount of memory allocated by nghttp2 internals
  uint64_t current_nghttp2_memory_ = 0;

  // The collection of active Http2Streams associated with this session
  std::unordered_map<int32_t, Http2Stream*> streams_;

  int flags_ = SESSION_STATE_NONE;

  // The StreamBase instance being used for i/o
  padding_strategy_type padding_strategy_ = PADDING_STRATEGY_NONE;

  // use this to allow timeout tracking during long-lasting writes
  uint32_t chunks_sent_since_last_write_ = 0;

  uv_buf_t stream_buf_ = uv_buf_init(nullptr, 0);
  v8::Global<v8::ArrayBuffer> stream_buf_ab_;
  AllocatedBuffer stream_buf_allocation_;

  size_t max_outstanding_pings_ = DEFAULT_MAX_PINGS;
  std::queue<std::unique_ptr<Http2Ping>> outstanding_pings_;

  size_t max_outstanding_settings_ = DEFAULT_MAX_SETTINGS;
  std::queue<std::unique_ptr<Http2Settings>> outstanding_settings_;

  std::vector<nghttp2_stream_write> outgoing_buffers_;
  std::vector<uint8_t> outgoing_storage_;
  std::vector<int32_t> pending_rst_streams_;
  // Count streams that have been rejected while being opened. Exceeding a fixed
  // limit will result in the session being destroyed, as an indication of a
  // misbehaving peer. This counter is reset once new streams are being
  // accepted again.
  int32_t rejected_stream_count_ = 0;
  // Also use the invalid frame count as a measure for rejecting input frames.
  int32_t invalid_frame_count_ = 0;

  void CopyDataIntoOutgoing(const uint8_t* src, size_t src_length);
  void ClearOutgoing(int status);

  friend class Http2Scope;
  friend class Http2StreamListener;
};

class Http2SessionPerformanceEntry : public PerformanceEntry {
 public:
  Http2SessionPerformanceEntry(
      Environment* env,
      const Http2Session::Statistics& stats,
      nghttp2_session_type type) :
          PerformanceEntry(env, "Http2Session", "http2",
                           stats.start_time,
                           stats.end_time),
          ping_rtt_(stats.ping_rtt),
          data_sent_(stats.data_sent),
          data_received_(stats.data_received),
          frame_count_(stats.frame_count),
          frame_sent_(stats.frame_sent),
          stream_count_(stats.stream_count),
          max_concurrent_streams_(stats.max_concurrent_streams),
          stream_average_duration_(stats.stream_average_duration),
          session_type_(type) { }

  uint64_t ping_rtt() const { return ping_rtt_; }