enum session_state_flags {
  SESSION_STATE_NONE = 0x0,
  SESSION_STATE_HAS_SCOPE = 0x1,
  SESSION_STATE_WRITE_SCHEDULED = 0x2,
  SESSION_STATE_CLOSED = 0x4,
  SESSION_STATE_CLOSING = 0x8,
  SESSION_STATE_SENDING = 0x10,
  SESSION_STATE_WRITE_IN_PROGRESS = 0x20,
  SESSION_STATE_READING_STOPPED = 0x40,
  SESSION_STATE_NGHTTP2_RECV_PAUSED = 0x80
};

typedef uint32_t(*get_setting)(nghttp2_session* session,
                               nghttp2_settings_id id);

class Http2Session;
class Http2Stream;

// This scope should be present when any call into nghttp2 that may schedule
// data to be written to the underlying transport is made, and schedules
// such a write automatically once the scope is exited.
class Http2Scope {
 public:
  explicit Http2Scope(Http2Stream* stream);
  explicit Http2Scope(Http2Session* session);
  ~Http2Scope();

 private:
  Http2Session* session_ = nullptr;
  Local<Object> session_handle_;
};

// The Http2Options class is used to parse the options object passed in to
// a Http2Session object and convert those into an appropriate nghttp2_option
// struct. This is the primary mechanism by which the Http2Session object is
// configured.
class Http2Options {
 public:
  Http2Options(Environment* env, nghttp2_session_type type);

  ~Http2Options() {
    nghttp2_option_del(options_);
  }

  nghttp2_option* operator*() const {
    return options_;
  }

  void SetMaxHeaderPairs(uint32_t max) {
    max_header_pairs_ = max;
  }

  uint32_t GetMaxHeaderPairs() const {
    return max_header_pairs_;
  }

  void SetPaddingStrategy(padding_strategy_type val) {
    padding_strategy_ = val;
  }

  padding_strategy_type GetPaddingStrategy() const {
    return padding_strategy_;
  }

  void SetMaxOutstandingPings(size_t max) {
    max_outstanding_pings_ = max;
  }

  size_t GetMaxOutstandingPings() {
    return max_outstanding_pings_;
  }

  void SetMaxOutstandingSettings(size_t max) {
    max_outstanding_settings_ = max;
  }

  size_t GetMaxOutstandingSettings() {
    return max_outstanding_settings_;
  }

  void SetMaxSessionMemory(uint64_t max) {
    max_session_memory_ = max;
  }

  uint64_t GetMaxSessionMemory() {
    return max_session_memory_;
  }

 private:
  nghttp2_option* options_;
  uint64_t max_session_memory_ = DEFAULT_MAX_SESSION_MEMORY;
  uint32_t max_header_pairs_ = DEFAULT_MAX_HEADER_LIST_PAIRS;
  padding_strategy_type padding_strategy_ = PADDING_STRATEGY_NONE;
  size_t max_outstanding_pings_ = DEFAULT_MAX_PINGS;
  size_t max_outstanding_settings_ = DEFAULT_MAX_SETTINGS;
};

class Http2Priority {
 public:
  Http2Priority(Environment* env,
                Local<Value> parent,
                Local<Value> weight,
                Local<Value> exclusive);

  nghttp2_priority_spec* operator*() {
    return &spec;
  }
 private:
  nghttp2_priority_spec spec;
};

class Http2StreamListener : public StreamListener {
 public:
  uv_buf_t OnStreamAlloc(size_t suggested_size) override;
  void OnStreamRead(ssize_t nread, const uv_buf_t& buf) override;
};

class Http2Stream : public AsyncWrap,
                    public StreamBase {
 public:
  static Http2Stream* New(
      Http2Session* session,
      int32_t id,
      nghttp2_headers_category category = NGHTTP2_HCAT_HEADERS,
      int options = 0);
  ~Http2Stream() override;

  nghttp2_stream* operator*();

  Http2Session* session() { return session_; }
  const Http2Session* session() const { return session_; }

  void EmitStatistics();

  // Required for StreamBase
  int ReadStart() override;

  // Required for StreamBase
  int ReadStop() override;

  // Required for StreamBase
  int DoShutdown(ShutdownWrap* req_wrap) override;

  bool HasWantsWrite() const override { return true; }

  // Initiate a response on this stream.
  int SubmitResponse(nghttp2_nv* nva, size_t len, int options);

  // Submit informational headers for this stream
  int SubmitInfo(nghttp2_nv* nva, size_t len);

  // Submit trailing headers for this stream
  int SubmitTrailers(nghttp2_nv* nva, size_t len);
  void OnTrailers();

  // Submit a PRIORITY frame for this stream
  int SubmitPriority(nghttp2_priority_spec* prispec, bool silent = false);

  // Submits an RST_STREAM frame using the given code
  void SubmitRstStream(const uint32_t code);

  void FlushRstStream();

  // Submits a PUSH_PROMISE frame with this stream as the parent.
  Http2Stream* SubmitPushPromise(
      nghttp2_nv* nva,
      size_t len,
      int32_t* ret,
      int options = 0);


  void Close(int32_t code);

  // Destroy this stream instance and free all held memory.
  void Destroy();

  inline bool IsDestroyed() const {
    return flags_ & NGHTTP2_STREAM_FLAG_DESTROYED;
  }

  inline bool IsWritable() const {
    return !(flags_ & NGHTTP2_STREAM_FLAG_SHUT);
  }

  inline bool IsPaused() const {
    return flags_ & NGHTTP2_STREAM_FLAG_READ_PAUSED;
  }

  inline bool IsClosed() const {
    return flags_ & NGHTTP2_STREAM_FLAG_CLOSED;
  }

  inline bool HasTrailers() const {
    return flags_ & NGHTTP2_STREAM_FLAG_TRAILERS;
  }

  // Returns true if this stream is in the reading state, which occurs when
  // the NGHTTP2_STREAM_FLAG_READ_START flag has been set and the
  // NGHTTP2_STREAM_FLAG_READ_PAUSED flag is *not* set.
  inline bool IsReading() const {
    return flags_ & NGHTTP2_STREAM_FLAG_READ_START &&
           !(flags_ & NGHTTP2_STREAM_FLAG_READ_PAUSED);
  }

  // Returns the RST_STREAM code used to close this stream
  inline int32_t code() const { return code_; }

  // Returns the stream identifier for this stream
  inline int32_t id() const { return id_; }
  inline bool IsDestroyed() {
    return (flags_ & SESSION_STATE_CLOSED) || session_ == nullptr;
  }

  // Schedule a write if nghttp2 indicates it wants to write to the socket.
  void MaybeScheduleWrite();

  // Stop reading if nghttp2 doesn't want to anymore.
  void MaybeStopReading();

  // Returns pointer to the stream, or nullptr if stream does not exist
  inline Http2Stream* FindStream(int32_t id);

  inline bool CanAddStream();

  // Adds a stream instance to this session
  inline void AddStream(Http2Stream* stream);

  // Removes a stream instance from this session
  inline void RemoveStream(Http2Stream* stream);

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
  void IncrementCurrentSessionMemory(uint64_t amount) {
    current_session_memory_ += amount;
  }

  void DecrementCurrentSessionMemory(uint64_t amount) {
    DCHECK_LE(amount, current_session_memory_);
    current_session_memory_ -= amount;
  }
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
  // When processing input data, either stream_buf_ab_ or stream_buf_allocation_
  // will be set. stream_buf_ab_ is lazily created from stream_buf_allocation_.
  v8::Global<v8::ArrayBuffer> stream_buf_ab_;
  AllocatedBuffer stream_buf_allocation_;
  size_t stream_buf_offset_ = 0;

  size_t max_outstanding_pings_ = DEFAULT_MAX_PINGS;
  std::queue<std::unique_ptr<Http2Ping>> outstanding_pings_;

  size_t max_outstanding_settings_ = DEFAULT_MAX_SETTINGS;
  std::queue<std::unique_ptr<Http2Settings>> outstanding_settings_;

  std::vector<nghttp2_stream_write> outgoing_buffers_;
  std::vector<uint8_t> outgoing_storage_;
  size_t outgoing_length_ = 0;
  std::vector<int32_t> pending_rst_streams_;
  // Count streams that have been rejected while being opened. Exceeding a fixed
  // limit will result in the session being destroyed, as an indication of a
  // misbehaving peer. This counter is reset once new streams are being
  // accepted again.
  int32_t rejected_stream_count_ = 0;
  // Also use the invalid frame count as a measure for rejecting input frames.
  int32_t invalid_frame_count_ = 0;

  void PushOutgoingBuffer(nghttp2_stream_write&& write);
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
  // When processing input data, either stream_buf_ab_ or stream_buf_allocation_
  // will be set. stream_buf_ab_ is lazily created from stream_buf_allocation_.
  v8::Global<v8::ArrayBuffer> stream_buf_ab_;
  AllocatedBuffer stream_buf_allocation_;
  size_t stream_buf_offset_ = 0;

  size_t max_outstanding_pings_ = DEFAULT_MAX_PINGS;
  std::queue<std::unique_ptr<Http2Ping>> outstanding_pings_;

  size_t max_outstanding_settings_ = DEFAULT_MAX_SETTINGS;
  std::queue<std::unique_ptr<Http2Settings>> outstanding_settings_;

  std::vector<nghttp2_stream_write> outgoing_buffers_;
  std::vector<uint8_t> outgoing_storage_;
  size_t outgoing_length_ = 0;
  std::vector<int32_t> pending_rst_streams_;
  // Count streams that have been rejected while being opened. Exceeding a fixed
  // limit will result in the session being destroyed, as an indication of a
  // misbehaving peer. This counter is reset once new streams are being
  // accepted again.
  int32_t rejected_stream_count_ = 0;
  // Also use the invalid frame count as a measure for rejecting input frames.
  int32_t invalid_frame_count_ = 0;

  void PushOutgoingBuffer(nghttp2_stream_write&& write);
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
  // When processing input data, either stream_buf_ab_ or stream_buf_allocation_
  // will be set. stream_buf_ab_ is lazily created from stream_buf_allocation_.
  v8::Global<v8::ArrayBuffer> stream_buf_ab_;
  AllocatedBuffer stream_buf_allocation_;
  size_t stream_buf_offset_ = 0;

  size_t max_outstanding_pings_ = DEFAULT_MAX_PINGS;
  std::queue<std::unique_ptr<Http2Ping>> outstanding_pings_;

  size_t max_outstanding_settings_ = DEFAULT_MAX_SETTINGS;
  std::queue<std::unique_ptr<Http2Settings>> outstanding_settings_;

  std::vector<nghttp2_stream_write> outgoing_buffers_;
  std::vector<uint8_t> outgoing_storage_;
  size_t outgoing_length_ = 0;
  std::vector<int32_t> pending_rst_streams_;
  // Count streams that have been rejected while being opened. Exceeding a fixed
  // limit will result in the session being destroyed, as an indication of a
  // misbehaving peer. This counter is reset once new streams are being
  // accepted again.
  int32_t rejected_stream_count_ = 0;
  // Also use the invalid frame count as a measure for rejecting input frames.
  int32_t invalid_frame_count_ = 0;

  void PushOutgoingBuffer(nghttp2_stream_write&& write);
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