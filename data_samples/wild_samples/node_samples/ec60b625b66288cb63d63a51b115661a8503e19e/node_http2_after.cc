  if (flags & (1 << IDX_OPTIONS_PEER_MAX_CONCURRENT_STREAMS)) {
    nghttp2_option_set_peer_max_concurrent_streams(
        options_,
        buffer[IDX_OPTIONS_PEER_MAX_CONCURRENT_STREAMS]);
  }

  if (IsReverted(SECURITY_REVERT_CVE_2019_9512))
    nghttp2_option_set_max_outbound_ack(options_, 10000);

  // The padding strategy sets the mechanism by which we determine how much
  // additional frame padding to apply to DATA and HEADERS frames. Currently
  // this is set on a per-session basis, but eventually we may switch to
  // a per-stream setting, giving users greater control
  if (flags & (1 << IDX_OPTIONS_PADDING_STRATEGY)) {
    padding_strategy_type strategy =
        static_cast<padding_strategy_type>(
            buffer.GetValue(IDX_OPTIONS_PADDING_STRATEGY));
    SetPaddingStrategy(strategy);
  }

  // The max header list pairs option controls the maximum number of
  // header pairs the session may accept. This is a hard limit.. that is,
  // if the remote peer sends more than this amount, the stream will be
  // automatically closed with an RST_STREAM.
  if (flags & (1 << IDX_OPTIONS_MAX_HEADER_LIST_PAIRS)) {
    SetMaxHeaderPairs(buffer[IDX_OPTIONS_MAX_HEADER_LIST_PAIRS]);
  }

  // The HTTP2 specification places no limits on the number of HTTP2
  // PING frames that can be sent. In order to prevent PINGS from being
  // abused as an attack vector, however, we place a strict upper limit
  // on the number of unacknowledged PINGS that can be sent at any given
  // time.
  if (flags & (1 << IDX_OPTIONS_MAX_OUTSTANDING_PINGS)) {
    SetMaxOutstandingPings(buffer[IDX_OPTIONS_MAX_OUTSTANDING_PINGS]);
  }

  // The HTTP2 specification places no limits on the number of HTTP2
  // SETTINGS frames that can be sent. In order to prevent PINGS from being
  // abused as an attack vector, however, we place a strict upper limit
  // on the number of unacknowledged SETTINGS that can be sent at any given
  // time.
  if (flags & (1 << IDX_OPTIONS_MAX_OUTSTANDING_SETTINGS)) {
    SetMaxOutstandingSettings(buffer[IDX_OPTIONS_MAX_OUTSTANDING_SETTINGS]);
  }

  // The HTTP2 specification places no limits on the amount of memory
  // that a session can consume. In order to prevent abuse, we place a
  // cap on the amount of memory a session can consume at any given time.
  // this is a credit based system. Existing streams may cause the limit
  // to be temporarily exceeded but once over the limit, new streams cannot
  // created.
  // Important: The maxSessionMemory option in javascript is expressed in
  //            terms of MB increments (i.e. the value 1 == 1 MB)
  if (flags & (1 << IDX_OPTIONS_MAX_SESSION_MEMORY)) {
    SetMaxSessionMemory(buffer[IDX_OPTIONS_MAX_SESSION_MEMORY] * 1e6);
  }
}

void Http2Session::Http2Settings::Init() {
  AliasedUint32Array& buffer = env()->http2_state()->settings_buffer;
  uint32_t flags = buffer[IDX_SETTINGS_COUNT];

  size_t n = 0;

#define GRABSETTING(N, trace)                                                 \
  if (flags & (1 << IDX_SETTINGS_##N)) {                                      \
    uint32_t val = buffer[IDX_SETTINGS_##N];                                  \
    if (session_ != nullptr)                                                  \
      Debug(session_, "setting " trace ": %d\n", val);                        \
    entries_[n++] =                                                           \
        nghttp2_settings_entry {NGHTTP2_SETTINGS_##N, val};                   \
  }

  GRABSETTING(HEADER_TABLE_SIZE, "header table size");
  GRABSETTING(MAX_CONCURRENT_STREAMS, "max concurrent streams");
  GRABSETTING(MAX_FRAME_SIZE, "max frame size");
  GRABSETTING(INITIAL_WINDOW_SIZE, "initial window size");
  GRABSETTING(MAX_HEADER_LIST_SIZE, "max header list size");
  GRABSETTING(ENABLE_PUSH, "enable push");
  GRABSETTING(ENABLE_CONNECT_PROTOCOL, "enable connect protocol");

#undef GRABSETTING

  count_ = n;
}

// The Http2Settings class is used to configure a SETTINGS frame that is
// to be sent to the connected peer. The settings are set using a TypedArray
// that is shared with the JavaScript side.
Http2Session::Http2Settings::Http2Settings(Environment* env,
                                           Http2Session* session,
                                           Local<Object> obj,
                                           uint64_t start_time)
    : AsyncWrap(env, obj, PROVIDER_HTTP2SETTINGS),
      session_(session),
      startTime_(start_time) {
  RemoveCleanupHook();  // This object is owned by the Http2Session.
  Init();
}

// Generates a Buffer that contains the serialized payload of a SETTINGS
// frame. This can be used, for instance, to create the Base64-encoded
// content of an Http2-Settings header field.
Local<Value> Http2Session::Http2Settings::Pack() {
  const size_t len = count_ * 6;
  Local<Value> buf = Buffer::New(env(), len).ToLocalChecked();
  ssize_t ret =
      nghttp2_pack_settings_payload(
        reinterpret_cast<uint8_t*>(Buffer::Data(buf)), len,
        &entries_[0], count_);
  if (ret >= 0)
    return buf;
  else
    return Undefined(env()->isolate());
}

// Updates the shared TypedArray with the current remote or local settings for
// the session.
void Http2Session::Http2Settings::Update(Environment* env,
                                         Http2Session* session,
                                         get_setting fn) {
  AliasedUint32Array& buffer = env->http2_state()->settings_buffer;
  buffer[IDX_SETTINGS_HEADER_TABLE_SIZE] =
      fn(**session, NGHTTP2_SETTINGS_HEADER_TABLE_SIZE);
  buffer[IDX_SETTINGS_MAX_CONCURRENT_STREAMS] =
      fn(**session, NGHTTP2_SETTINGS_MAX_CONCURRENT_STREAMS);
  buffer[IDX_SETTINGS_INITIAL_WINDOW_SIZE] =
      fn(**session, NGHTTP2_SETTINGS_INITIAL_WINDOW_SIZE);
  buffer[IDX_SETTINGS_MAX_FRAME_SIZE] =
      fn(**session, NGHTTP2_SETTINGS_MAX_FRAME_SIZE);
  buffer[IDX_SETTINGS_MAX_HEADER_LIST_SIZE] =
      fn(**session, NGHTTP2_SETTINGS_MAX_HEADER_LIST_SIZE);
  buffer[IDX_SETTINGS_ENABLE_PUSH] =
      fn(**session, NGHTTP2_SETTINGS_ENABLE_PUSH);
  buffer[IDX_SETTINGS_ENABLE_CONNECT_PROTOCOL] =
      fn(**session, NGHTTP2_SETTINGS_ENABLE_CONNECT_PROTOCOL);
}

// Initializes the shared TypedArray with the default settings values.
void Http2Session::Http2Settings::RefreshDefaults(Environment* env) {
  AliasedUint32Array& buffer = env->http2_state()->settings_buffer;

  buffer[IDX_SETTINGS_HEADER_TABLE_SIZE] =
      DEFAULT_SETTINGS_HEADER_TABLE_SIZE;
  buffer[IDX_SETTINGS_ENABLE_PUSH] =
      DEFAULT_SETTINGS_ENABLE_PUSH;
  buffer[IDX_SETTINGS_INITIAL_WINDOW_SIZE] =
      DEFAULT_SETTINGS_INITIAL_WINDOW_SIZE;
  buffer[IDX_SETTINGS_MAX_FRAME_SIZE] =
      DEFAULT_SETTINGS_MAX_FRAME_SIZE;
  buffer[IDX_SETTINGS_MAX_HEADER_LIST_SIZE] =
      DEFAULT_SETTINGS_MAX_HEADER_LIST_SIZE;
  buffer[IDX_SETTINGS_COUNT] =
    (1 << IDX_SETTINGS_HEADER_TABLE_SIZE) |
    (1 << IDX_SETTINGS_ENABLE_PUSH) |
    (1 << IDX_SETTINGS_INITIAL_WINDOW_SIZE) |
    (1 << IDX_SETTINGS_MAX_FRAME_SIZE) |
    (1 << IDX_SETTINGS_MAX_HEADER_LIST_SIZE);
}


void Http2Session::Http2Settings::Send() {
  Http2Scope h2scope(session_);
  CHECK_EQ(nghttp2_submit_settings(**session_, NGHTTP2_FLAG_NONE,
                                   &entries_[0], count_), 0);
}

void Http2Session::Http2Settings::Done(bool ack) {
  uint64_t end = uv_hrtime();
  double duration = (end - startTime_) / 1e6;

  Local<Value> argv[] = {
    Boolean::New(env()->isolate(), ack),
    Number::New(env()->isolate(), duration)
  };
  MakeCallback(env()->ondone_string(), arraysize(argv), argv);
}

// The Http2Priority class initializes an appropriate nghttp2_priority_spec
// struct used when either creating a stream or updating its priority
// settings.
Http2Priority::Http2Priority(Environment* env,
                             Local<Value> parent,
                             Local<Value> weight,
                             Local<Value> exclusive) {
  Local<Context> context = env->context();
  int32_t parent_ = parent->Int32Value(context).ToChecked();
  int32_t weight_ = weight->Int32Value(context).ToChecked();
  bool exclusive_ = exclusive->BooleanValue(env->isolate());
  Debug(env, DebugCategory::HTTP2STREAM,
        "Http2Priority: parent: %d, weight: %d, exclusive: %d\n",
        parent_, weight_, exclusive_);
  nghttp2_priority_spec_init(&spec, parent_, weight_, exclusive_ ? 1 : 0);
}


const char* Http2Session::TypeName() const {
  switch (session_type_) {
    case NGHTTP2_SESSION_SERVER: return "server";
    case NGHTTP2_SESSION_CLIENT: return "client";
    default:
      // This should never happen
      ABORT();
  }
}

// The Headers class initializes a proper array of nghttp2_nv structs
// containing the header name value pairs.
Headers::Headers(Isolate* isolate,
                 Local<Context> context,
                 Local<Array> headers) {
  Local<Value> header_string = headers->Get(context, 0).ToLocalChecked();
  Local<Value> header_count = headers->Get(context, 1).ToLocalChecked();
  count_ = header_count.As<Uint32>()->Value();
  int header_string_len = header_string.As<String>()->Length();

  if (count_ == 0) {
    CHECK_EQ(header_string_len, 0);
    return;
  }

  // Allocate a single buffer with count_ nghttp2_nv structs, followed
  // by the raw header data as passed from JS. This looks like:
  // | possible padding | nghttp2_nv | nghttp2_nv | ... | header contents |
  buf_.AllocateSufficientStorage((alignof(nghttp2_nv) - 1) +
                                 count_ * sizeof(nghttp2_nv) +
                                 header_string_len);
  // Make sure the start address is aligned appropriately for an nghttp2_nv*.
  char* start = reinterpret_cast<char*>(
      RoundUp(reinterpret_cast<uintptr_t>(*buf_), alignof(nghttp2_nv)));
  char* header_contents = start + (count_ * sizeof(nghttp2_nv));
  nghttp2_nv* const nva = reinterpret_cast<nghttp2_nv*>(start);

  CHECK_LE(header_contents + header_string_len, *buf_ + buf_.length());
  CHECK_EQ(header_string.As<String>()->WriteOneByte(
               isolate,
               reinterpret_cast<uint8_t*>(header_contents),
               0,
               header_string_len,
               String::NO_NULL_TERMINATION),
           header_string_len);

  size_t n = 0;
  char* p;
  for (p = header_contents; p < header_contents + header_string_len; n++) {
    if (n >= count_) {
      // This can happen if a passed header contained a null byte. In that
      // case, just provide nghttp2 with an invalid header to make it reject
      // the headers list.
      static uint8_t zero = '\0';
      nva[0].name = nva[0].value = &zero;
      nva[0].namelen = nva[0].valuelen = 1;
      count_ = 1;
      return;
    }