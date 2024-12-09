  if (flags & (1 << IDX_OPTIONS_PADDING_STRATEGY)) {
    PaddingStrategy strategy =
        static_cast<PaddingStrategy>(
            buffer.GetValue(IDX_OPTIONS_PADDING_STRATEGY));
    set_padding_strategy(strategy);
  }

  // The max header list pairs option controls the maximum number of
  // header pairs the session may accept. This is a hard limit.. that is,
  // if the remote peer sends more than this amount, the stream will be
  // automatically closed with an RST_STREAM.
  if (flags & (1 << IDX_OPTIONS_MAX_HEADER_LIST_PAIRS))
    set_max_header_pairs(buffer[IDX_OPTIONS_MAX_HEADER_LIST_PAIRS]);

  // The HTTP2 specification places no limits on the number of HTTP2
  // PING frames that can be sent. In order to prevent PINGS from being
  // abused as an attack vector, however, we place a strict upper limit
  // on the number of unacknowledged PINGS that can be sent at any given
  // time.
  if (flags & (1 << IDX_OPTIONS_MAX_OUTSTANDING_PINGS))
    set_max_outstanding_pings(buffer[IDX_OPTIONS_MAX_OUTSTANDING_PINGS]);

  // The HTTP2 specification places no limits on the number of HTTP2
  // SETTINGS frames that can be sent. In order to prevent PINGS from being
  // abused as an attack vector, however, we place a strict upper limit
  // on the number of unacknowledged SETTINGS that can be sent at any given
  // time.
  if (flags & (1 << IDX_OPTIONS_MAX_OUTSTANDING_SETTINGS))
    set_max_outstanding_settings(buffer[IDX_OPTIONS_MAX_OUTSTANDING_SETTINGS]);

  // The HTTP2 specification places no limits on the amount of memory
  // that a session can consume. In order to prevent abuse, we place a
  // cap on the amount of memory a session can consume at any given time.
  // this is a credit based system. Existing streams may cause the limit
  // to be temporarily exceeded but once over the limit, new streams cannot
  // created.
  // Important: The maxSessionMemory option in javascript is expressed in
  //            terms of MB increments (i.e. the value 1 == 1 MB)
  if (flags & (1 << IDX_OPTIONS_MAX_SESSION_MEMORY))
    set_max_session_memory(buffer[IDX_OPTIONS_MAX_SESSION_MEMORY] * 1000000);
}

#define GRABSETTING(entries, count, name)                                      \
  do {                                                                         \
    if (flags & (1 << IDX_SETTINGS_ ## name)) {                                \
      uint32_t val = buffer[IDX_SETTINGS_ ## name];                            \
      entries[count++] =                                                       \
          nghttp2_settings_entry {NGHTTP2_SETTINGS_ ## name, val};             \
    } } while (0)

size_t Http2Settings::Init(
    Http2State* http2_state,
    nghttp2_settings_entry* entries) {
  AliasedUint32Array& buffer = http2_state->settings_buffer;
  uint32_t flags = buffer[IDX_SETTINGS_COUNT];

  size_t count = 0;

#define V(name) GRABSETTING(entries, count, name);
  HTTP2_SETTINGS(V)
#undef V

  return count;
}
#undef GRABSETTING

// The Http2Settings class is used to configure a SETTINGS frame that is
// to be sent to the connected peer. The settings are set using a TypedArray
// that is shared with the JavaScript side.
Http2Settings::Http2Settings(Http2Session* session,
                             Local<Object> obj,
                             Local<Function> callback,
                             uint64_t start_time)
    : AsyncWrap(session->env(), obj, PROVIDER_HTTP2SETTINGS),
      session_(session),
      startTime_(start_time) {
  callback_.Reset(env()->isolate(), callback);
  count_ = Init(session->http2_state(), entries_);
}

Local<Function> Http2Settings::callback() const {
  return callback_.Get(env()->isolate());
}

void Http2Settings::MemoryInfo(MemoryTracker* tracker) const {
  tracker->TrackField("callback", callback_);
}

// Generates a Buffer that contains the serialized payload of a SETTINGS
// frame. This can be used, for instance, to create the Base64-encoded
// content of an Http2-Settings header field.
Local<Value> Http2Settings::Pack() {
  return Pack(session_->env(), count_, entries_);
}

Local<Value> Http2Settings::Pack(Http2State* state) {
  nghttp2_settings_entry entries[IDX_SETTINGS_COUNT];
  size_t count = Init(state, entries);
  return Pack(state->env(), count, entries);
}

Local<Value> Http2Settings::Pack(
    Environment* env,
    size_t count,
    const nghttp2_settings_entry* entries) {
  EscapableHandleScope scope(env->isolate());
  const size_t size = count * 6;
  AllocatedBuffer buffer = AllocatedBuffer::AllocateManaged(env, size);
  ssize_t ret =
      nghttp2_pack_settings_payload(
          reinterpret_cast<uint8_t*>(buffer.data()),
          size,
          entries,
          count);
  Local<Value> buf = Undefined(env->isolate());
  if (ret >= 0) buf = buffer.ToBuffer().ToLocalChecked();
  return scope.Escape(buf);
}

// Updates the shared TypedArray with the current remote or local settings for
// the session.
void Http2Settings::Update(Http2Session* session, get_setting fn) {
  AliasedUint32Array& buffer = session->http2_state()->settings_buffer;

#define V(name)                                                                \
  buffer[IDX_SETTINGS_ ## name] =                                              \
      fn(session->session(), NGHTTP2_SETTINGS_ ## name);
  HTTP2_SETTINGS(V)
#undef V
}

// Initializes the shared TypedArray with the default settings values.
void Http2Settings::RefreshDefaults(Http2State* http2_state) {
  AliasedUint32Array& buffer = http2_state->settings_buffer;
  uint32_t flags = 0;

#define V(name)                                                            \
  do {                                                                     \
    buffer[IDX_SETTINGS_ ## name] = DEFAULT_SETTINGS_ ## name;             \
    flags |= 1 << IDX_SETTINGS_ ## name;                                   \
  } while (0);
  HTTP2_SETTINGS(V)
#undef V

  buffer[IDX_SETTINGS_COUNT] = flags;
}


void Http2Settings::Send() {
  Http2Scope h2scope(session_.get());
  CHECK_EQ(nghttp2_submit_settings(
      session_->session(),
      NGHTTP2_FLAG_NONE,
      &entries_[0],
      count_), 0);
}

void Http2Settings::Done(bool ack) {
  uint64_t end = uv_hrtime();
  double duration = (end - startTime_) / 1e6;

  Local<Value> argv[] = {
    ack ? v8::True(env()->isolate()) : v8::False(env()->isolate()),
    Number::New(env()->isolate(), duration)
  };
  MakeCallback(callback(), arraysize(argv), argv);
}