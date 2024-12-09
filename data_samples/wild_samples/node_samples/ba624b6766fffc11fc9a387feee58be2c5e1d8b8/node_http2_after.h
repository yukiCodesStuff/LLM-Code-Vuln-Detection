enum session_state_flags {
  SESSION_STATE_NONE = 0x0,
  SESSION_STATE_HAS_SCOPE = 0x1,
  SESSION_STATE_WRITE_SCHEDULED = 0x2,
  SESSION_STATE_CLOSED = 0x4,
  SESSION_STATE_CLOSING = 0x8,
  SESSION_STATE_SENDING = 0x10,
  SESSION_STATE_WRITE_IN_PROGRESS = 0x20,
  SESSION_STATE_READING_STOPPED = 0x40,
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