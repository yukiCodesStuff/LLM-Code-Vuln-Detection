  if (nread < 0) {
    PassReadErrorToPreviousListener(nread);
    return;
  }

  CHECK(!session->stream_buf_ab_.IsEmpty());

  // There is a single large array buffer for the entire data read from the
  // network; create a slice of that array buffer and emit it as the
  // received data buffer.
  size_t offset = buf.base - session->stream_buf_.base;

  // Verify that the data offset is inside the current read buffer.
  CHECK_LE(offset, session->stream_buf_.len);
  CHECK_LE(offset + buf.len, session->stream_buf_.len);

  stream->CallJSOnreadMethod(nread, session->stream_buf_ab_, offset);
}


// Called by OnFrameReceived to notify JavaScript land that a complete
// HEADERS frame has been received and processed. This method converts the
// received headers into a JavaScript array and pushes those out to JS.
void Http2Session::HandleHeadersFrame(const nghttp2_frame* frame) {
  Isolate* isolate = env()->isolate();
  HandleScope scope(isolate);
  Local<Context> context = env()->context();
  Context::Scope context_scope(context);

  int32_t id = GetFrameID(frame);
  Debug(this, "handle headers frame for stream %d", id);
  Http2Stream* stream = FindStream(id);

  // If the stream has already been destroyed, ignore.
  if (stream->IsDestroyed())
    return;

  std::vector<nghttp2_header> headers(stream->move_headers());

  // The headers are passed in above as a queue of nghttp2_header structs.
  // The following converts that into a JS array with the structure:
  // [name1, value1, name2, value2, name3, value3, name3, value4] and so on.
  // That array is passed up to the JS layer and converted into an Object form
  // like {name1: value1, name2: value2, name3: [value3, value4]}. We do it
  // this way for performance reasons (it's faster to generate and pass an
  // array than it is to generate and pass the object).
  size_t headers_size = headers.size();
  std::vector<Local<Value>> headers_v(headers_size * 2);
  for (size_t i = 0; i < headers_size; ++i) {
    const nghttp2_header& item = headers[i];
    // The header name and value are passed as external one-byte strings
    headers_v[i * 2] =
        ExternalHeader::New<true>(this, item.name).ToLocalChecked();
    headers_v[i * 2 + 1] =
        ExternalHeader::New<false>(this, item.value).ToLocalChecked();
  }

  Local<Value> args[5] = {
      stream->object(),
      Integer::New(isolate, id),
      Integer::New(isolate, stream->headers_category()),
      Integer::New(isolate, frame->hd.flags),
      Array::New(isolate, headers_v.data(), headers_size * 2)};
  MakeCallback(env()->http2session_on_headers_function(),
               arraysize(args), args);
}


// Called by OnFrameReceived when a complete PRIORITY frame has been
// received. Notifies JS land about the priority change. Note that priorities
// are considered advisory only, so this has no real effect other than to
// simply let user code know that the priority has changed.
void Http2Session::HandlePriorityFrame(const nghttp2_frame* frame) {
  if (js_fields_[kSessionPriorityListenerCount] == 0) return;
  Isolate* isolate = env()->isolate();
  HandleScope scope(isolate);
  Local<Context> context = env()->context();
  Context::Scope context_scope(context);

  nghttp2_priority priority_frame = frame->priority;
  int32_t id = GetFrameID(frame);
  Debug(this, "handle priority frame for stream %d", id);
  // Priority frame stream ID should never be <= 0. nghttp2 handles this for us
  nghttp2_priority_spec spec = priority_frame.pri_spec;

  Local<Value> argv[4] = {
    Integer::New(isolate, id),
    Integer::New(isolate, spec.stream_id),
    Integer::New(isolate, spec.weight),
    Boolean::New(isolate, spec.exclusive)
  };
  MakeCallback(env()->http2session_on_priority_function(),
               arraysize(argv), argv);
}
  if (nread < 0) {
    PassReadErrorToPreviousListener(nread);
    return;
  }

  CHECK(!session->stream_buf_ab_.IsEmpty());

  // There is a single large array buffer for the entire data read from the
  // network; create a slice of that array buffer and emit it as the
  // received data buffer.
  size_t offset = buf.base - session->stream_buf_.base;

  // Verify that the data offset is inside the current read buffer.
  CHECK_LE(offset, session->stream_buf_.len);
  CHECK_LE(offset + buf.len, session->stream_buf_.len);

  stream->CallJSOnreadMethod(nread, session->stream_buf_ab_, offset);
}


// Called by OnFrameReceived to notify JavaScript land that a complete
// HEADERS frame has been received and processed. This method converts the
// received headers into a JavaScript array and pushes those out to JS.
void Http2Session::HandleHeadersFrame(const nghttp2_frame* frame) {
  Isolate* isolate = env()->isolate();
  HandleScope scope(isolate);
  Local<Context> context = env()->context();
  Context::Scope context_scope(context);

  int32_t id = GetFrameID(frame);
  Debug(this, "handle headers frame for stream %d", id);
  Http2Stream* stream = FindStream(id);

  // If the stream has already been destroyed, ignore.
  if (stream->IsDestroyed())
    return;

  std::vector<nghttp2_header> headers(stream->move_headers());

  // The headers are passed in above as a queue of nghttp2_header structs.
  // The following converts that into a JS array with the structure:
  // [name1, value1, name2, value2, name3, value3, name3, value4] and so on.
  // That array is passed up to the JS layer and converted into an Object form
  // like {name1: value1, name2: value2, name3: [value3, value4]}. We do it
  // this way for performance reasons (it's faster to generate and pass an
  // array than it is to generate and pass the object).
  size_t headers_size = headers.size();
  std::vector<Local<Value>> headers_v(headers_size * 2);
  for (size_t i = 0; i < headers_size; ++i) {
    const nghttp2_header& item = headers[i];
    // The header name and value are passed as external one-byte strings
    headers_v[i * 2] =
        ExternalHeader::New<true>(this, item.name).ToLocalChecked();
    headers_v[i * 2 + 1] =
        ExternalHeader::New<false>(this, item.value).ToLocalChecked();
  }
void Http2Session::OnStreamRead(ssize_t nread, const uv_buf_t& buf_) {
  HandleScope handle_scope(env()->isolate());
  Context::Scope context_scope(env()->context());
  Http2Scope h2scope(this);
  CHECK_NOT_NULL(stream_);
  Debug(this, "receiving %d bytes", nread);
  CHECK(stream_buf_ab_.IsEmpty());
  AllocatedBuffer buf(env(), buf_);

  // Only pass data on if nread > 0
  if (nread <= 0) {
    if (nread < 0) {
      PassReadErrorToPreviousListener(nread);
    }
    return;
  }
  OnScopeLeave on_scope_leave([&]() {
    // Once finished handling this write, reset the stream buffer.
    // The memory has either been free()d or was handed over to V8.
    // We use `nread` instead of `buf.size()` here, because the buffer is
    // cleared as part of the `.ToArrayBuffer()` call below.
    DecrementCurrentSessionMemory(nread);
    stream_buf_ab_ = Local<ArrayBuffer>();
    stream_buf_ = uv_buf_init(nullptr, 0);
  });

  // Make sure that there was no read previously active.
  CHECK_NULL(stream_buf_.base);
  CHECK_EQ(stream_buf_.len, 0);

  // Remember the current buffer, so that OnDataChunkReceived knows the
  // offset of a DATA frame's data into the socket read buffer.
  stream_buf_ = uv_buf_init(buf.data(), nread);

  Isolate* isolate = env()->isolate();

  // Create an array buffer for the read data. DATA frames will be emitted
  // as slices of this array buffer to avoid having to copy memory.
  stream_buf_ab_ = buf.ToArrayBuffer();

  statistics_.data_received += nread;
  ssize_t ret = Write(&stream_buf_, 1);

  if (UNLIKELY(ret < 0)) {
    Debug(this, "fatal error receiving data: %d", ret);
    Local<Value> arg = Integer::New(isolate, ret);
    MakeCallback(env()->http2session_on_error_function(), 1, &arg);
    return;
  }
  OnScopeLeave on_scope_leave([&]() {
    // Once finished handling this write, reset the stream buffer.
    // The memory has either been free()d or was handed over to V8.
    // We use `nread` instead of `buf.size()` here, because the buffer is
    // cleared as part of the `.ToArrayBuffer()` call below.
    DecrementCurrentSessionMemory(nread);
    stream_buf_ab_ = Local<ArrayBuffer>();
    stream_buf_ = uv_buf_init(nullptr, 0);
  });

  // Make sure that there was no read previously active.
  CHECK_NULL(stream_buf_.base);
  CHECK_EQ(stream_buf_.len, 0);

  // Remember the current buffer, so that OnDataChunkReceived knows the
  // offset of a DATA frame's data into the socket read buffer.
  stream_buf_ = uv_buf_init(buf.data(), nread);

  Isolate* isolate = env()->isolate();

  // Create an array buffer for the read data. DATA frames will be emitted
  // as slices of this array buffer to avoid having to copy memory.
  stream_buf_ab_ = buf.ToArrayBuffer();

  statistics_.data_received += nread;
  ssize_t ret = Write(&stream_buf_, 1);

  if (UNLIKELY(ret < 0)) {
    Debug(this, "fatal error receiving data: %d", ret);
    Local<Value> arg = Integer::New(isolate, ret);
    MakeCallback(env()->http2session_on_error_function(), 1, &arg);
    return;
  }