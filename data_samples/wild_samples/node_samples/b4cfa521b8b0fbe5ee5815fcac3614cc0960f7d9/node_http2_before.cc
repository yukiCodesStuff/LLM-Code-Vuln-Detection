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
  for (nghttp2_header& header : current_headers_) {
    nghttp2_rcbuf_decref(header.name);
    nghttp2_rcbuf_decref(header.value);
  }

  if (session_ == nullptr)
    return;
  Debug(this, "tearing down stream");
  session_->RemoveStream(this);
  session_ = nullptr;
}

std::string Http2Stream::diagnostic_name() const {
  return "HttpStream " + std::to_string(id()) + " (" +
      std::to_string(static_cast<int64_t>(get_async_id())) + ") [" +
      session()->diagnostic_name() + "]";
}

// Notify the Http2Stream that a new block of HEADERS is being processed.
void Http2Stream::StartHeaders(nghttp2_headers_category category) {
  Debug(this, "starting headers, category: %d", id_, category);
  CHECK(!this->IsDestroyed());
  current_headers_length_ = 0;
  current_headers_.clear();
  current_headers_category_ = category;
}


nghttp2_stream* Http2Stream::operator*() {
  return nghttp2_session_find_stream(**session_, id_);
}

void Http2Stream::Close(int32_t code) {
  CHECK(!this->IsDestroyed());
  flags_ |= NGHTTP2_STREAM_FLAG_CLOSED;
  code_ = code;
  Debug(this, "closed with code %d", code);
}

int Http2Stream::DoShutdown(ShutdownWrap* req_wrap) {
  if (IsDestroyed())
    return UV_EPIPE;

  {
    Http2Scope h2scope(this);
    flags_ |= NGHTTP2_STREAM_FLAG_SHUT;
    CHECK_NE(nghttp2_session_resume_data(**session_, id_),
             NGHTTP2_ERR_NOMEM);
    Debug(this, "writable side shutdown");
  }
void Http2Stream::StartHeaders(nghttp2_headers_category category) {
  Debug(this, "starting headers, category: %d", id_, category);
  CHECK(!this->IsDestroyed());
  current_headers_length_ = 0;
  current_headers_.clear();
  current_headers_category_ = category;
}


nghttp2_stream* Http2Stream::operator*() {
  return nghttp2_session_find_stream(**session_, id_);
}

void Http2Stream::Close(int32_t code) {
  CHECK(!this->IsDestroyed());
  flags_ |= NGHTTP2_STREAM_FLAG_CLOSED;
  code_ = code;
  Debug(this, "closed with code %d", code);
}

int Http2Stream::DoShutdown(ShutdownWrap* req_wrap) {
  if (IsDestroyed())
    return UV_EPIPE;

  {
    Http2Scope h2scope(this);
    flags_ |= NGHTTP2_STREAM_FLAG_SHUT;
    CHECK_NE(nghttp2_session_resume_data(**session_, id_),
             NGHTTP2_ERR_NOMEM);
    Debug(this, "writable side shutdown");
  }
                            uint8_t flags) {
  CHECK(!this->IsDestroyed());
  if (this->statistics_.first_header == 0)
    this->statistics_.first_header = uv_hrtime();
  size_t length = nghttp2_rcbuf_get_buf(name).len +
                  nghttp2_rcbuf_get_buf(value).len + 32;
  // A header can only be added if we have not exceeded the maximum number
  // of headers and the session has memory available for it.
  if (!session_->IsAvailableSessionMemory(length) ||
      current_headers_.size() == max_header_pairs_ ||
      current_headers_length_ + length > max_header_length_) {
    return false;
  }
      current_headers_length_ + length > max_header_length_) {
    return false;
  }
  nghttp2_header header;
  header.name = name;
  header.value = value;
  header.flags = flags;
  current_headers_.push_back(header);
  nghttp2_rcbuf_incref(name);
  nghttp2_rcbuf_incref(value);
  current_headers_length_ += length;
  return true;
}

// A Provider is the thing that provides outbound DATA frame data.
Http2Stream::Provider::Provider(Http2Stream* stream, int options) {
  CHECK(!stream->IsDestroyed());
  provider_.source.ptr = stream;
  empty_ = options & STREAM_OPTION_EMPTY_PAYLOAD;
}

Http2Stream::Provider::Provider(int options) {
  provider_.source.ptr = nullptr;
  empty_ = options & STREAM_OPTION_EMPTY_PAYLOAD;
}

Http2Stream::Provider::~Provider() {
  provider_.source.ptr = nullptr;
}

// The Stream Provider pulls data from a linked list of uv_buf_t structs
// built via the StreamBase API and the Streams js API.
Http2Stream::Provider::Stream::Stream(int options)
    : Http2Stream::Provider(options) {
  provider_.read_callback = Http2Stream::Provider::Stream::OnRead;
}

Http2Stream::Provider::Stream::Stream(Http2Stream* stream, int options)
    : Http2Stream::Provider(stream, options) {
  provider_.read_callback = Http2Stream::Provider::Stream::OnRead;
}

ssize_t Http2Stream::Provider::Stream::OnRead(nghttp2_session* handle,
                                              int32_t id,
                                              uint8_t* buf,
                                              size_t length,
                                              uint32_t* flags,
                                              nghttp2_data_source* source,
                                              void* user_data) {
  Http2Session* session = static_cast<Http2Session*>(user_data);
  Debug(session, "reading outbound data for stream %d", id);
  Http2Stream* stream = GetStream(session, id, source);
  if (stream->statistics_.first_byte_sent == 0)
    stream->statistics_.first_byte_sent = uv_hrtime();
  CHECK_EQ(id, stream->id());

  size_t amount = 0;          // amount of data being sent in this data frame.

  // Remove all empty chunks from the head of the queue.
  // This is done here so that .write('', cb) is still a meaningful way to
  // find out when the HTTP2 stream wants to consume data, and because the
  // StreamBase API allows empty input chunks.
  while (!stream->queue_.empty() && stream->queue_.front().buf.len == 0) {
    WriteWrap* finished = stream->queue_.front().req_wrap;
    stream->queue_.pop();
    if (finished != nullptr)
      finished->Done(0);
  }