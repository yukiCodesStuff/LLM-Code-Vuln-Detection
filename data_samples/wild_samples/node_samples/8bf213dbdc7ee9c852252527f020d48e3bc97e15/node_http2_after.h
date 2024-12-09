  struct Statistics {
    uint64_t start_time;
    uint64_t end_time;
    uint64_t first_header;     // Time first header was received
    uint64_t first_byte;       // Time first DATA frame byte was received
    uint64_t first_byte_sent;  // Time first DATA frame byte was sent
    uint64_t sent_bytes;
    uint64_t received_bytes;
  };

  Statistics statistics_ = {};

 private:
  Http2Session* session_ = nullptr;             // The Parent HTTP/2 Session
  int32_t id_ = 0;                              // The Stream Identifier
  int32_t code_ = NGHTTP2_NO_ERROR;             // The RST_STREAM code (if any)
  int flags_ = NGHTTP2_STREAM_FLAG_NONE;        // Internal state flags

  uint32_t max_header_pairs_ = DEFAULT_MAX_HEADER_LIST_PAIRS;
  uint32_t max_header_length_ = DEFAULT_SETTINGS_MAX_HEADER_LIST_SIZE;

  // The Current Headers block... As headers are received for this stream,
  // they are temporarily stored here until the OnFrameReceived is called
  // signalling the end of the HEADERS frame
  nghttp2_headers_category current_headers_category_ = NGHTTP2_HCAT_HEADERS;
  uint32_t current_headers_length_ = 0;  // total number of octets
  std::vector<nghttp2_header> current_headers_;

  // This keeps track of the amount of data read from the socket while the
  // socket was in paused mode. When `ReadStart()` is called (and not before
  // then), we tell nghttp2 that we consumed that data to get proper
  // backpressure handling.
  size_t inbound_consumed_data_while_paused_ = 0;

  // Outbound Data... This is the data written by the JS layer that is
  // waiting to be written out to the socket.
  std::queue<nghttp2_stream_write> queue_;
  size_t available_outbound_length_ = 0;

  Http2StreamListener stream_listener_;

  friend class Http2Session;
};

class Http2Stream::Provider {
 public:
  Provider(Http2Stream* stream, int options);
  explicit Provider(int options);
  virtual ~Provider();

  nghttp2_data_provider* operator*() {
    return !empty_ ? &provider_ : nullptr;
  }

  class FD;
  class Stream;
 protected:
  nghttp2_data_provider provider_;

 private:
  bool empty_ = false;
};

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


class Http2Session : public AsyncWrap, public StreamListener {
 public:
  Http2Session(Environment* env,
               Local<Object> wrap,
               nghttp2_session_type type = NGHTTP2_SESSION_SERVER);
  ~Http2Session() override;

  class Http2Ping;
  class Http2Settings;

  void EmitStatistics();

  inline StreamBase* underlying_stream() {
    return static_cast<StreamBase*>(stream_);
  }

  void Start();
  void Stop();
  void Close(uint32_t code = NGHTTP2_NO_ERROR,
             bool socket_closed = false);
  void Consume(Local<External> external);
  void Unconsume();
  void Goaway(uint32_t code, int32_t lastStreamID, uint8_t* data, size_t len);
  void AltSvc(int32_t id,
              uint8_t* origin,
              size_t origin_len,
              uint8_t* value,
              size_t value_len);

  bool Ping(v8::Local<v8::Function> function);

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