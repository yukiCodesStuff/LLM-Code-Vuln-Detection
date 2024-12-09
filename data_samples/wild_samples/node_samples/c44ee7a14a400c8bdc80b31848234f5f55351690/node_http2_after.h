class Http2Stream::Provider::Stream : public Http2Stream::Provider {
 public:
  Stream(Http2Stream* stream, int options);
  explicit Stream(int options);

  static ssize_t OnRead(nghttp2_session* session,
                        int32_t id,
                        uint8_t* buf,
                        size_t length,
                        uint32_t* flags,
                        nghttp2_data_source* source,
                        void* user_data);
};

// Indices for js_fields_, which serves as a way to communicate data with JS
// land fast. In particular, we store information about the number/presence
// of certain event listeners in JS, and skip calls from C++ into JS if they
// are missing.
enum SessionUint8Fields {
  kBitfield,  // See below
  kSessionPriorityListenerCount,
  kSessionFrameErrorListenerCount,
  kSessionUint8FieldCount
};

enum SessionBitfieldFlags {
  kSessionHasRemoteSettingsListeners,
  kSessionRemoteSettingsIsUpToDate,
  kSessionHasPingListeners,
  kSessionHasAltsvcListeners
};

class Http2Session : public AsyncWrap, public StreamListener {
 public:
  Http2Session(Environment* env,
               Local<Object> wrap,
               nghttp2_session_type type = NGHTTP2_SESSION_SERVER);
  ~Http2Session() override;

  class Http2Ping;
  class Http2Settings;
  class MemoryAllocatorInfo;

  void EmitStatistics();

  inline StreamBase* underlying_stream() {
    return static_cast<StreamBase*>(stream_);
  }

  void Close(uint32_t code = NGHTTP2_NO_ERROR,
             bool socket_closed = false);
  void Consume(Local<Object> stream);
  void Goaway(uint32_t code, int32_t lastStreamID,
              const uint8_t* data, size_t len);
  void AltSvc(int32_t id,
              uint8_t* origin,
              size_t origin_len,
              uint8_t* value,
              size_t value_len);
  void Origin(nghttp2_origin_entry* ov, size_t count);

  uint8_t SendPendingData();

  // Submits a new request. If the request is a success, assigned
  // will be a pointer to the Http2Stream instance assigned.
  // This only works if the session is a client session.
  Http2Stream* SubmitRequest(
      nghttp2_priority_spec* prispec,
      nghttp2_nv* nva,
      size_t len,
      int32_t* ret,
      int options = 0);

  inline nghttp2_session_type type() const { return session_type_; }

  inline nghttp2_session* session() const { return session_; }

  inline nghttp2_session* operator*() { return session_; }
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
  v8::Local<v8::ArrayBuffer> stream_buf_ab_;

  size_t max_outstanding_pings_ = DEFAULT_MAX_PINGS;
  std::queue<std::unique_ptr<Http2Ping>> outstanding_pings_;

  size_t max_outstanding_settings_ = DEFAULT_MAX_SETTINGS;
  std::queue<std::unique_ptr<Http2Settings>> outstanding_settings_;

  std::vector<nghttp2_stream_write> outgoing_buffers_;
  std::vector<uint8_t> outgoing_storage_;
  std::vector<int32_t> pending_rst_streams_;

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