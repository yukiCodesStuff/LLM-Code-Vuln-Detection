  switch (frame->hd.type) {
    case NGHTTP2_DATA:
      return session->HandleDataFrame(frame);
    case NGHTTP2_PUSH_PROMISE:
      // Intentional fall-through, handled just like headers frames
    case NGHTTP2_HEADERS:
      session->HandleHeadersFrame(frame);
      break;
    case NGHTTP2_SETTINGS:
      session->HandleSettingsFrame(frame);
      break;
    case NGHTTP2_PRIORITY:
      session->HandlePriorityFrame(frame);
      break;
    case NGHTTP2_GOAWAY:
      session->HandleGoawayFrame(frame);
      break;
    case NGHTTP2_PING:
      session->HandlePingFrame(frame);
      break;
    case NGHTTP2_ALTSVC:
      session->HandleAltSvcFrame(frame);
      break;
    case NGHTTP2_ORIGIN:
      session->HandleOriginFrame(frame);
      break;
    default:
      break;
  }
  Local<Value> argv[4] = {
    Integer::New(isolate, id),
    Integer::New(isolate, spec.stream_id),
    Integer::New(isolate, spec.weight),
    Boolean::New(isolate, spec.exclusive)
  };
  MakeCallback(env()->http2session_on_priority_function(),
               arraysize(argv), argv);
}


// Called by OnFrameReceived when a complete DATA frame has been received.
// If we know that this was the last DATA frame (because the END_STREAM flag
// is set), then we'll terminate the readable side of the StreamBase.
int Http2Session::HandleDataFrame(const nghttp2_frame* frame) {
  int32_t id = GetFrameID(frame);
  Debug(this, "handling data frame for stream %d", id);
  Http2Stream* stream = FindStream(id);

  if (!stream->IsDestroyed() && frame->hd.flags & NGHTTP2_FLAG_END_STREAM) {
    stream->EmitRead(UV_EOF);
  } else if (frame->hd.length == 0 &&
           !IsReverted(SECURITY_REVERT_CVE_2019_9518)) {
    return 1;  // Consider 0-length frame without END_STREAM an error.
  }
  return 0;
}


// Called by OnFrameReceived when a complete GOAWAY frame has been received.
void Http2Session::HandleGoawayFrame(const nghttp2_frame* frame) {
  Isolate* isolate = env()->isolate();
  HandleScope scope(isolate);
  Local<Context> context = env()->context();
  Context::Scope context_scope(context);

  nghttp2_goaway goaway_frame = frame->goaway;
  Debug(this, "handling goaway frame");

  Local<Value> argv[3] = {
    Integer::NewFromUnsigned(isolate, goaway_frame.error_code),
    Integer::New(isolate, goaway_frame.last_stream_id),
    Undefined(isolate)
  };

  size_t length = goaway_frame.opaque_data_len;
  if (length > 0) {
    argv[2] = Buffer::Copy(isolate,
                           reinterpret_cast<char*>(goaway_frame.opaque_data),
                           length).ToLocalChecked();
  }

  MakeCallback(env()->http2session_on_goaway_data_function(),
               arraysize(argv), argv);
}

// Called by OnFrameReceived when a complete ALTSVC frame has been received.
void Http2Session::HandleAltSvcFrame(const nghttp2_frame* frame) {
  if (!(js_fields_[kBitfield] & (1 << kSessionHasAltsvcListeners))) return;
  Isolate* isolate = env()->isolate();
  HandleScope scope(isolate);
  Local<Context> context = env()->context();
  Context::Scope context_scope(context);

  int32_t id = GetFrameID(frame);

  nghttp2_extension ext = frame->ext;
  nghttp2_ext_altsvc* altsvc = static_cast<nghttp2_ext_altsvc*>(ext.payload);
  Debug(this, "handling altsvc frame");

  Local<Value> argv[3] = {
    Integer::New(isolate, id),
    String::NewFromOneByte(isolate,
                           altsvc->origin,
                           NewStringType::kNormal,
                           altsvc->origin_len).ToLocalChecked(),
    String::NewFromOneByte(isolate,
                           altsvc->field_value,
                           NewStringType::kNormal,
                           altsvc->field_value_len).ToLocalChecked(),
  };

  MakeCallback(env()->http2session_on_altsvc_function(),
               arraysize(argv), argv);
}

void Http2Session::HandleOriginFrame(const nghttp2_frame* frame) {
  Isolate* isolate = env()->isolate();
  HandleScope scope(isolate);
  Local<Context> context = env()->context();
  Context::Scope context_scope(context);

  Debug(this, "handling origin frame");

  nghttp2_extension ext = frame->ext;
  nghttp2_ext_origin* origin = static_cast<nghttp2_ext_origin*>(ext.payload);

  size_t nov = origin->nov;
  std::vector<Local<Value>> origin_v(nov);

  for (size_t i = 0; i < nov; ++i) {
    const nghttp2_origin_entry& entry = origin->ov[i];
    origin_v[i] =
        String::NewFromOneByte(
            isolate, entry.origin, NewStringType::kNormal, entry.origin_len)
            .ToLocalChecked();
  }
  Local<Value> holder = Array::New(isolate, origin_v.data(), origin_v.size());
  MakeCallback(env()->http2session_on_origin_function(), 1, &holder);
}

// Called by OnFrameReceived when a complete PING frame has been received.
void Http2Session::HandlePingFrame(const nghttp2_frame* frame) {
  Isolate* isolate = env()->isolate();
  HandleScope scope(isolate);
  Local<Context> context = env()->context();
  Context::Scope context_scope(context);
  Local<Value> arg;
  bool ack = frame->hd.flags & NGHTTP2_FLAG_ACK;
  if (ack) {
    std::unique_ptr<Http2Ping> ping = PopPing();

    if (!ping) {
      // PING Ack is unsolicited. Treat as a connection error. The HTTP/2
      // spec does not require this, but there is no legitimate reason to
      // receive an unsolicited PING ack on a connection. Either the peer
      // is buggy or malicious, and we're not going to tolerate such
      // nonsense.
      arg = Integer::New(isolate, NGHTTP2_ERR_PROTO);
      MakeCallback(env()->http2session_on_error_function(), 1, &arg);
      return;
    }

    ping->Done(true, frame->ping.opaque_data);
    return;
  }

  if (!(js_fields_[kBitfield] & (1 << kSessionHasPingListeners))) return;
  // Notify the session that a ping occurred
  arg = Buffer::Copy(env(),
                      reinterpret_cast<const char*>(frame->ping.opaque_data),
                      8).ToLocalChecked();
  MakeCallback(env()->http2session_on_ping_function(), 1, &arg);
}

// Called by OnFrameReceived when a complete SETTINGS frame has been received.
void Http2Session::HandleSettingsFrame(const nghttp2_frame* frame) {
  bool ack = frame->hd.flags & NGHTTP2_FLAG_ACK;
  if (!ack) {
    js_fields_[kBitfield] &= ~(1 << kSessionRemoteSettingsIsUpToDate);
    if (!(js_fields_[kBitfield] & (1 << kSessionHasRemoteSettingsListeners)))
      return;
    // This is not a SETTINGS acknowledgement, notify and return
    MakeCallback(env()->http2session_on_settings_function(), 0, nullptr);
    return;
  }

  // If this is an acknowledgement, we should have an Http2Settings
  // object for it.
  std::unique_ptr<Http2Settings> settings = PopSettings();
  if (settings) {
    settings->Done(true);
    return;
  }
  // SETTINGS Ack is unsolicited. Treat as a connection error. The HTTP/2
  // spec does not require this, but there is no legitimate reason to
  // receive an unsolicited SETTINGS ack on a connection. Either the peer
  // is buggy or malicious, and we're not going to tolerate such
  // nonsense.
  // Note that nghttp2 currently prevents this from happening for SETTINGS
  // frames, so this block is purely defensive just in case that behavior
  // changes. Specifically, unlike unsolicited PING acks, unsolicited
  // SETTINGS acks should *never* make it this far.
  Isolate* isolate = env()->isolate();
  HandleScope scope(isolate);
  Local<Context> context = env()->context();
  Context::Scope context_scope(context);
  Local<Value> arg = Integer::New(isolate, NGHTTP2_ERR_PROTO);
  MakeCallback(env()->http2session_on_error_function(), 1, &arg);
}

// Callback used when data has been written to the stream.
void Http2Session::OnStreamAfterWrite(WriteWrap* w, int status) {
  Debug(this, "write finished with status %d", status);

  // Inform all pending writes about their completion.
  ClearOutgoing(status);

  if (!(flags_ & SESSION_STATE_WRITE_SCHEDULED)) {
    // Schedule a new write if nghttp2 wants to send data.
    MaybeScheduleWrite();
  }
}

// If the underlying nghttp2_session struct has data pending in its outbound
// queue, MaybeScheduleWrite will schedule a SendPendingData() call to occur
// on the next iteration of the Node.js event loop (using the SetImmediate
// queue), but only if a write has not already been scheduled.
void Http2Session::MaybeScheduleWrite() {
  CHECK_EQ(flags_ & SESSION_STATE_WRITE_SCHEDULED, 0);
  if (UNLIKELY(session_ == nullptr))
    return;

  if (nghttp2_session_want_write(session_)) {
    HandleScope handle_scope(env()->isolate());
    Debug(this, "scheduling write");
    flags_ |= SESSION_STATE_WRITE_SCHEDULED;
    env()->SetImmediate([this](Environment* env) {
      if (session_ == nullptr || !(flags_ & SESSION_STATE_WRITE_SCHEDULED)) {
        // This can happen e.g. when a stream was reset before this turn
        // of the event loop, in which case SendPendingData() is called early,
        // or the session was destroyed in the meantime.
        return;
      }

      // Sending data may call arbitrary JS code, so keep track of
      // async context.
      HandleScope handle_scope(env->isolate());
      InternalCallbackScope callback_scope(this);
      SendPendingData();
    }, object());
  }
}

void Http2Session::MaybeStopReading() {
  int want_read = nghttp2_session_want_read(session_);
  Debug(this, "wants read? %d", want_read);
  if (want_read == 0)
    stream_->ReadStop();
}

// Unset the sending state, finish up all current writes, and reset
// storage for data and metadata that was associated with these writes.
void Http2Session::ClearOutgoing(int status) {
  CHECK_NE(flags_ & SESSION_STATE_SENDING, 0);

  flags_ &= ~SESSION_STATE_SENDING;

  if (outgoing_buffers_.size() > 0) {
    outgoing_storage_.clear();

    std::vector<nghttp2_stream_write> current_outgoing_buffers_;
    current_outgoing_buffers_.swap(outgoing_buffers_);
    for (const nghttp2_stream_write& wr : current_outgoing_buffers_) {
      WriteWrap* wrap = wr.req_wrap;
      if (wrap != nullptr) {
        // TODO(addaleax): Pass `status` instead of 0, so that we actually error
        // out with the error from the write to the underlying protocol,
        // if one occurred.
        wrap->Done(0);
      }
    }
  }

  // Now that we've finished sending queued data, if there are any pending
  // RstStreams we should try sending again and then flush them one by one.
  if (pending_rst_streams_.size() > 0) {
    std::vector<int32_t> current_pending_rst_streams;
    pending_rst_streams_.swap(current_pending_rst_streams);

    SendPendingData();

    for (int32_t stream_id : current_pending_rst_streams) {
      Http2Stream* stream = FindStream(stream_id);
      if (LIKELY(stream != nullptr))
        stream->FlushRstStream();
    }
  }
}

// Queue a given block of data for sending. This always creates a copy,
// so it is used for the cases in which nghttp2 requests sending of a
// small chunk of data.
void Http2Session::CopyDataIntoOutgoing(const uint8_t* src, size_t src_length) {
  size_t offset = outgoing_storage_.size();
  outgoing_storage_.resize(offset + src_length);
  memcpy(&outgoing_storage_[offset], src, src_length);

  // Store with a base of `nullptr` initially, since future resizes
  // of the outgoing_buffers_ vector may invalidate the pointer.
  // The correct base pointers will be set later, before writing to the
  // underlying socket.
  outgoing_buffers_.emplace_back(nghttp2_stream_write {
    uv_buf_init(nullptr, src_length)
  });
}

// Prompts nghttp2 to begin serializing it's pending data and pushes each
// chunk out to the i/o socket to be sent. This is a particularly hot method
// that will generally be called at least twice be event loop iteration.
// This is a potential performance optimization target later.
// Returns non-zero value if a write is already in progress.
uint8_t Http2Session::SendPendingData() {
  Debug(this, "sending pending data");
  // Do not attempt to send data on the socket if the destroying flag has
  // been set. That means everything is shutting down and the socket
  // will not be usable.
  if (IsDestroyed())
    return 0;
  flags_ &= ~SESSION_STATE_WRITE_SCHEDULED;

  // SendPendingData should not be called recursively.
  if (flags_ & SESSION_STATE_SENDING)
    return 1;
  // This is cleared by ClearOutgoing().
  flags_ |= SESSION_STATE_SENDING;

  ssize_t src_length;
  const uint8_t* src;

  CHECK_EQ(outgoing_buffers_.size(), 0);
  CHECK_EQ(outgoing_storage_.size(), 0);

  // Part One: Gather data from nghttp2

  while ((src_length = nghttp2_session_mem_send(session_, &src)) > 0) {
    Debug(this, "nghttp2 has %d bytes to send", src_length);
    CopyDataIntoOutgoing(src, src_length);
  }

  CHECK_NE(src_length, NGHTTP2_ERR_NOMEM);

  if (stream_ == nullptr) {
    // It would seem nice to bail out earlier, but `nghttp2_session_mem_send()`
    // does take care of things like closing the individual streams after
    // a socket has been torn down, so we still need to call it.
    ClearOutgoing(UV_ECANCELED);
    return 0;
  }

  // Part Two: Pass Data to the underlying stream

  size_t count = outgoing_buffers_.size();
  if (count == 0) {
    ClearOutgoing(0);
    return 0;
  }
  MaybeStackBuffer<uv_buf_t, 32> bufs;
  bufs.AllocateSufficientStorage(count);

  // Set the buffer base pointers for copied data that ended up in the
  // sessions's own storage since it might have shifted around during gathering.
  // (Those are marked by having .base == nullptr.)
  size_t offset = 0;
  size_t i = 0;
  for (const nghttp2_stream_write& write : outgoing_buffers_) {
    statistics_.data_sent += write.buf.len;
    if (write.buf.base == nullptr) {
      bufs[i++] = uv_buf_init(
          reinterpret_cast<char*>(outgoing_storage_.data() + offset),
          write.buf.len);
      offset += write.buf.len;
    } else {
      bufs[i++] = write.buf;
    }
  }

  chunks_sent_since_last_write_++;

  StreamWriteResult res = underlying_stream()->Write(*bufs, count);
  if (!res.async) {
    ClearOutgoing(res.err);
  }

  MaybeStopReading();

  return 0;
}


// This callback is called from nghttp2 when it wants to send DATA frames for a
// given Http2Stream, when we set the `NGHTTP2_DATA_FLAG_NO_COPY` flag earlier
// in the Http2Stream::Provider::Stream::OnRead callback.
// We take the write information directly out of the stream's data queue.
int Http2Session::OnSendData(
      nghttp2_session* session_,
      nghttp2_frame* frame,
      const uint8_t* framehd,
      size_t length,
      nghttp2_data_source* source,
      void* user_data) {
  Http2Session* session = static_cast<Http2Session*>(user_data);
  Http2Stream* stream = GetStream(session, frame->hd.stream_id, source);

  // Send the frame header + a byte that indicates padding length.
  session->CopyDataIntoOutgoing(framehd, 9);
  if (frame->data.padlen > 0) {
    uint8_t padding_byte = frame->data.padlen - 1;
    CHECK_EQ(padding_byte, frame->data.padlen - 1);
    session->CopyDataIntoOutgoing(&padding_byte, 1);
  }

  Debug(session, "nghttp2 has %d bytes to send directly", length);
  while (length > 0) {
    // nghttp2 thinks that there is data available (length > 0), which means
    // we told it so, which means that we *should* have data available.
    CHECK(!stream->queue_.empty());

    nghttp2_stream_write& write = stream->queue_.front();
    if (write.buf.len <= length) {
      // This write does not suffice by itself, so we can consume it completely.
      length -= write.buf.len;
      session->outgoing_buffers_.emplace_back(std::move(write));
      stream->queue_.pop();
      continue;
    }

    // Slice off `length` bytes of the first write in the queue.
    session->outgoing_buffers_.emplace_back(nghttp2_stream_write {
      uv_buf_init(write.buf.base, length)
    });
    write.buf.base += length;
    write.buf.len -= length;
    break;
  }

  if (frame->data.padlen > 0) {
    // Send padding if that was requested.
    session->outgoing_buffers_.emplace_back(nghttp2_stream_write {
      uv_buf_init(const_cast<char*>(zero_bytes_256), frame->data.padlen - 1)
    });
  }

  return 0;
}

// Creates a new Http2Stream and submits a new http2 request.
Http2Stream* Http2Session::SubmitRequest(
    nghttp2_priority_spec* prispec,
    nghttp2_nv* nva,
    size_t len,
    int32_t* ret,
    int options) {
  Debug(this, "submitting request");
  Http2Scope h2scope(this);
  Http2Stream* stream = nullptr;
  Http2Stream::Provider::Stream prov(options);
  *ret = nghttp2_submit_request(session_, prispec, nva, len, *prov, nullptr);
  CHECK_NE(*ret, NGHTTP2_ERR_NOMEM);
  if (LIKELY(*ret > 0))
    stream = Http2Stream::New(this, *ret, NGHTTP2_HCAT_HEADERS, options);
  return stream;
}

uv_buf_t Http2Session::OnStreamAlloc(size_t suggested_size) {
  return env()->AllocateManaged(suggested_size).release();
}

// Callback used to receive inbound data from the i/o stream
void Http2Session::OnStreamRead(ssize_t nread, const uv_buf_t& buf_) {
  HandleScope handle_scope(env()->isolate());
  Context::Scope context_scope(env()->context());
  Http2Scope h2scope(this);
  CHECK_NOT_NULL(stream_);
  Debug(this, "receiving %d bytes", nread);
  CHECK_EQ(stream_buf_allocation_.size(), 0);
  CHECK(stream_buf_ab_.IsEmpty());
  AllocatedBuffer buf(env(), buf_);

  // Only pass data on if nread > 0
  if (nread <= 0) {
    if (nread < 0) {
      PassReadErrorToPreviousListener(nread);
    }
    return;
  }

  // Shrink to the actual amount of used data.
  buf.Resize(nread);

  IncrementCurrentSessionMemory(nread);
  OnScopeLeave on_scope_leave([&]() {
    // Once finished handling this write, reset the stream buffer.
    // The memory has either been free()d or was handed over to V8.
    // We use `nread` instead of `buf.size()` here, because the buffer is
    // cleared as part of the `.ToArrayBuffer()` call below.
    DecrementCurrentSessionMemory(nread);
    stream_buf_ab_.Reset();
    stream_buf_allocation_.clear();
    stream_buf_ = uv_buf_init(nullptr, 0);
  });

  // Make sure that there was no read previously active.
  CHECK_NULL(stream_buf_.base);
  CHECK_EQ(stream_buf_.len, 0);

  // Remember the current buffer, so that OnDataChunkReceived knows the
  // offset of a DATA frame's data into the socket read buffer.
  stream_buf_ = uv_buf_init(buf.data(), nread);

  Isolate* isolate = env()->isolate();

  // Store this so we can create an ArrayBuffer for read data from it.
  // DATA frames will be emitted as slices of that ArrayBuffer to avoid having
  // to copy memory.
  stream_buf_allocation_ = std::move(buf);

  statistics_.data_received += nread;
  ssize_t ret = Write(&stream_buf_, 1);

  if (UNLIKELY(ret < 0)) {
    Debug(this, "fatal error receiving data: %d", ret);
    Local<Value> arg = Integer::New(isolate, ret);
    MakeCallback(env()->http2session_on_error_function(), 1, &arg);
    return;
  }

  MaybeStopReading();
}

bool Http2Session::HasWritesOnSocketForStream(Http2Stream* stream) {
  for (const nghttp2_stream_write& wr : outgoing_buffers_) {
    if (wr.req_wrap != nullptr && wr.req_wrap->stream() == stream)
      return true;
  }
  return false;
}

// Every Http2Session session is tightly bound to a single i/o StreamBase
// (typically a net.Socket or tls.TLSSocket). The lifecycle of the two is
// tightly coupled with all data transfer between the two happening at the
// C++ layer via the StreamBase API.
void Http2Session::Consume(Local<Object> stream_obj) {
  StreamBase* stream = StreamBase::FromObject(stream_obj);
  stream->PushStreamListener(this);
  Debug(this, "i/o stream consumed");
}

Http2Stream* Http2Stream::New(Http2Session* session,
                              int32_t id,
                              nghttp2_headers_category category,
                              int options) {
  Local<Object> obj;
  if (!session->env()
           ->http2stream_constructor_template()
           ->NewInstance(session->env()->context())
           .ToLocal(&obj)) {
    return nullptr;
  }
  return new Http2Stream(session, obj, id, category, options);
}

Http2Stream::Http2Stream(Http2Session* session,
                         Local<Object> obj,
                         int32_t id,
                         nghttp2_headers_category category,
                         int options)
    : AsyncWrap(session->env(), obj, AsyncWrap::PROVIDER_HTTP2STREAM),
      StreamBase(session->env()),
      session_(session),
      id_(id),
      current_headers_category_(category) {
  MakeWeak();
  StreamBase::AttachToObject(GetObject());
  statistics_.start_time = uv_hrtime();

  // Limit the number of header pairs
  max_header_pairs_ = session->GetMaxHeaderPairs();
  if (max_header_pairs_ == 0) {
    max_header_pairs_ = DEFAULT_MAX_HEADER_LIST_PAIRS;
  }
  current_headers_.reserve(std::min(max_header_pairs_, 12u));

  // Limit the number of header octets
  max_header_length_ =
      std::min(
        nghttp2_session_get_local_settings(
          session->session(),
          NGHTTP2_SETTINGS_MAX_HEADER_LIST_SIZE),
      MAX_MAX_HEADER_LIST_SIZE);

  if (options & STREAM_OPTION_GET_TRAILERS)
    flags_ |= NGHTTP2_STREAM_FLAG_TRAILERS;

  PushStreamListener(&stream_listener_);

  if (options & STREAM_OPTION_EMPTY_PAYLOAD)
    Shutdown();
  session->AddStream(this);
}


Http2Stream::~Http2Stream() {
  for (nghttp2_header& header : current_headers_) {
    nghttp2_rcbuf_decref(header.name);
    nghttp2_rcbuf_decref(header.value);
  }

  if (session_ == nullptr)
    return;
  Debug(this, "tearing down stream");
  session_->DecrementCurrentSessionMemory(current_headers_length_);
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
  session_->DecrementCurrentSessionMemory(current_headers_length_);
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
  return 1;
}

// Destroy the Http2Stream and render it unusable. Actual resources for the
// Stream will not be freed until the next tick of the Node.js event loop
// using the SetImmediate queue.
void Http2Stream::Destroy() {
  // Do nothing if this stream instance is already destroyed
  if (IsDestroyed())
    return;
  if (session_->HasPendingRstStream(id_))
    FlushRstStream();
  flags_ |= NGHTTP2_STREAM_FLAG_DESTROYED;

  Debug(this, "destroying stream");

  // Wait until the start of the next loop to delete because there
  // may still be some pending operations queued for this stream.
  env()->SetImmediate([this](Environment* env) {
    // Free any remaining outgoing data chunks here. This should be done
    // here because it's possible for destroy to have been called while
    // we still have queued outbound writes.
    while (!queue_.empty()) {
      nghttp2_stream_write& head = queue_.front();
      if (head.req_wrap != nullptr)
        head.req_wrap->Done(UV_ECANCELED);
      queue_.pop();
    }

    // We can destroy the stream now if there are no writes for it
    // already on the socket. Otherwise, we'll wait for the garbage collector
    // to take care of cleaning up.
    if (session() == nullptr || !session()->HasWritesOnSocketForStream(this))
      delete this;
  }, object());

  statistics_.end_time = uv_hrtime();
  session_->statistics_.stream_average_duration =
      ((statistics_.end_time - statistics_.start_time) /
          session_->statistics_.stream_count) / 1e6;
  EmitStatistics();
}


// Initiates a response on the Http2Stream using data provided via the
// StreamBase Streams API.
int Http2Stream::SubmitResponse(nghttp2_nv* nva, size_t len, int options) {
  CHECK(!this->IsDestroyed());
  Http2Scope h2scope(this);
  Debug(this, "submitting response");
  if (options & STREAM_OPTION_GET_TRAILERS)
    flags_ |= NGHTTP2_STREAM_FLAG_TRAILERS;

  if (!IsWritable())
    options |= STREAM_OPTION_EMPTY_PAYLOAD;

  Http2Stream::Provider::Stream prov(this, options);
  int ret = nghttp2_submit_response(**session_, id_, nva, len, *prov);
  CHECK_NE(ret, NGHTTP2_ERR_NOMEM);
  return ret;
}


// Submit informational headers for a stream.
int Http2Stream::SubmitInfo(nghttp2_nv* nva, size_t len) {
  CHECK(!this->IsDestroyed());
  Http2Scope h2scope(this);
  Debug(this, "sending %d informational headers", len);
  int ret = nghttp2_submit_headers(**session_,
                                   NGHTTP2_FLAG_NONE,
                                   id_, nullptr,
                                   nva, len, nullptr);
  CHECK_NE(ret, NGHTTP2_ERR_NOMEM);
  return ret;
}

void Http2Stream::OnTrailers() {
  Debug(this, "let javascript know we are ready for trailers");
  CHECK(!this->IsDestroyed());
  Isolate* isolate = env()->isolate();
  HandleScope scope(isolate);
  Local<Context> context = env()->context();
  Context::Scope context_scope(context);
  flags_ &= ~NGHTTP2_STREAM_FLAG_TRAILERS;
  MakeCallback(env()->http2session_on_stream_trailers_function(), 0, nullptr);
}

// Submit informational headers for a stream.
int Http2Stream::SubmitTrailers(nghttp2_nv* nva, size_t len) {
  CHECK(!this->IsDestroyed());
  Http2Scope h2scope(this);
  Debug(this, "sending %d trailers", len);
  int ret;
  // Sending an empty trailers frame poses problems in Safari, Edge & IE.
  // Instead we can just send an empty data frame with NGHTTP2_FLAG_END_STREAM
  // to indicate that the stream is ready to be closed.
  if (len == 0) {
    Http2Stream::Provider::Stream prov(this, 0);
    ret = nghttp2_submit_data(**session_, NGHTTP2_FLAG_END_STREAM, id_, *prov);
  } else {
    ret = nghttp2_submit_trailer(**session_, id_, nva, len);
  }
  CHECK_NE(ret, NGHTTP2_ERR_NOMEM);
  return ret;
}

// Submit a PRIORITY frame to the connected peer.
int Http2Stream::SubmitPriority(nghttp2_priority_spec* prispec,
                                bool silent) {
  CHECK(!this->IsDestroyed());
  Http2Scope h2scope(this);
  Debug(this, "sending priority spec");
  int ret = silent ?
      nghttp2_session_change_stream_priority(**session_,
                                             id_, prispec) :
      nghttp2_submit_priority(**session_,
                              NGHTTP2_FLAG_NONE,
                              id_, prispec);
  CHECK_NE(ret, NGHTTP2_ERR_NOMEM);
  return ret;
}

// Closes the Http2Stream by submitting an RST_STREAM frame to the connected
// peer.
void Http2Stream::SubmitRstStream(const uint32_t code) {
  CHECK(!this->IsDestroyed());
  code_ = code;
  // If possible, force a purge of any currently pending data here to make sure
  // it is sent before closing the stream. If it returns non-zero then we need
  // to wait until the current write finishes and try again to avoid nghttp2
  // behaviour where it prioritizes RstStream over everything else.
  if (session_->SendPendingData() != 0) {
    session_->AddPendingRstStream(id_);
    return;
  }

  FlushRstStream();
}

void Http2Stream::FlushRstStream() {
  if (IsDestroyed())
    return;
  Http2Scope h2scope(this);
  CHECK_EQ(nghttp2_submit_rst_stream(**session_, NGHTTP2_FLAG_NONE,
                                     id_, code_), 0);
}


// Submit a push promise and create the associated Http2Stream if successful.
Http2Stream* Http2Stream::SubmitPushPromise(nghttp2_nv* nva,
                                            size_t len,
                                            int32_t* ret,
                                            int options) {
  CHECK(!this->IsDestroyed());
  Http2Scope h2scope(this);
  Debug(this, "sending push promise");
  *ret = nghttp2_submit_push_promise(**session_, NGHTTP2_FLAG_NONE,
                                     id_, nva, len, nullptr);
  CHECK_NE(*ret, NGHTTP2_ERR_NOMEM);
  Http2Stream* stream = nullptr;
  if (*ret > 0)
    stream = Http2Stream::New(session_, *ret, NGHTTP2_HCAT_HEADERS, options);

  return stream;
}

// Switch the StreamBase into flowing mode to begin pushing chunks of data
// out to JS land.
int Http2Stream::ReadStart() {
  Http2Scope h2scope(this);
  CHECK(!this->IsDestroyed());
  flags_ |= NGHTTP2_STREAM_FLAG_READ_START;
  flags_ &= ~NGHTTP2_STREAM_FLAG_READ_PAUSED;

  Debug(this, "reading starting");

  // Tell nghttp2 about our consumption of the data that was handed
  // off to JS land.
  nghttp2_session_consume_stream(**session_,
                                 id_,
                                 inbound_consumed_data_while_paused_);
  inbound_consumed_data_while_paused_ = 0;

  return 0;
}

// Switch the StreamBase into paused mode.
int Http2Stream::ReadStop() {
  CHECK(!this->IsDestroyed());
  if (!IsReading())
    return 0;
  flags_ |= NGHTTP2_STREAM_FLAG_READ_PAUSED;
  Debug(this, "reading stopped");
  return 0;
}

// The Http2Stream class is a subclass of StreamBase. The DoWrite method
// receives outbound chunks of data to send as outbound DATA frames. These
// are queued in an internal linked list of uv_buf_t structs that are sent
// when nghttp2 is ready to serialize the data frame.
//
// Queue the given set of uv_but_t handles for writing to an
// nghttp2_stream. The WriteWrap's Done callback will be invoked once the
// chunks of data have been flushed to the underlying nghttp2_session.
// Note that this does *not* mean that the data has been flushed
// to the socket yet.
int Http2Stream::DoWrite(WriteWrap* req_wrap,
                         uv_buf_t* bufs,
                         size_t nbufs,
                         uv_stream_t* send_handle) {
  CHECK_NULL(send_handle);
  Http2Scope h2scope(this);
  if (!IsWritable() || IsDestroyed()) {
    req_wrap->Done(UV_EOF);
    return 0;
  }
  Debug(this, "queuing %d buffers to send", id_, nbufs);
  for (size_t i = 0; i < nbufs; ++i) {
    // Store the req_wrap on the last write info in the queue, so that it is
    // only marked as finished once all buffers associated with it are finished.
    queue_.emplace(nghttp2_stream_write {
      i == nbufs - 1 ? req_wrap : nullptr,
      bufs[i]
    });
    IncrementAvailableOutboundLength(bufs[i].len);
  }
  CHECK_NE(nghttp2_session_resume_data(**session_, id_), NGHTTP2_ERR_NOMEM);
  return 0;
}

// Ads a header to the Http2Stream. Note that the header name and value are
// provided using a buffer structure provided by nghttp2 that allows us to
// avoid unnecessary memcpy's. Those buffers are ref counted. The ref count
// is incremented here and are decremented when the header name and values
// are garbage collected later.
bool Http2Stream::AddHeader(nghttp2_rcbuf* name,
                            nghttp2_rcbuf* value,
                            uint8_t flags) {
  CHECK(!this->IsDestroyed());
  if (this->statistics_.first_header == 0)
    this->statistics_.first_header = uv_hrtime();
  size_t name_len = nghttp2_rcbuf_get_buf(name).len;
  if (name_len == 0 && !IsReverted(SECURITY_REVERT_CVE_2019_9516)) {
    return true;  // Ignore headers with empty names.
  }
  size_t value_len = nghttp2_rcbuf_get_buf(value).len;
  size_t length = name_len + value_len + 32;
  // A header can only be added if we have not exceeded the maximum number
  // of headers and the session has memory available for it.
  if (!session_->IsAvailableSessionMemory(length) ||
      current_headers_.size() == max_header_pairs_ ||
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
  session_->IncrementCurrentSessionMemory(length);
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

  if (!stream->queue_.empty()) {
    Debug(session, "stream %d has pending outbound data", id);
    amount = std::min(stream->available_outbound_length_, length);
    Debug(session, "sending %d bytes for data frame on stream %d", amount, id);
    if (amount > 0) {
      // Just return the length, let Http2Session::OnSendData take care of
      // actually taking the buffers out of the queue.
      *flags |= NGHTTP2_DATA_FLAG_NO_COPY;
      stream->DecrementAvailableOutboundLength(amount);
    }
  }

  if (amount == 0 && stream->IsWritable()) {
    CHECK(stream->queue_.empty());
    Debug(session, "deferring stream %d", id);
    stream->EmitWantsWrite(length);
    if (stream->available_outbound_length_ > 0 || !stream->IsWritable()) {
      // EmitWantsWrite() did something interesting synchronously, restart:
      return OnRead(handle, id, buf, length, flags, source, user_data);
    }
    return NGHTTP2_ERR_DEFERRED;
  }

  if (stream->queue_.empty() && !stream->IsWritable()) {
    Debug(session, "no more data for stream %d", id);
    *flags |= NGHTTP2_DATA_FLAG_EOF;
    if (stream->HasTrailers()) {
      *flags |= NGHTTP2_DATA_FLAG_NO_END_STREAM;
      stream->OnTrailers();
    }
  }

  stream->statistics_.sent_bytes += amount;
  return amount;
}

inline void Http2Stream::IncrementAvailableOutboundLength(size_t amount) {
  available_outbound_length_ += amount;
  session_->IncrementCurrentSessionMemory(amount);
}

inline void Http2Stream::DecrementAvailableOutboundLength(size_t amount) {
  available_outbound_length_ -= amount;
  session_->DecrementCurrentSessionMemory(amount);
}


// Implementation of the JavaScript API

// Fetches the string description of a nghttp2 error code and passes that
// back to JS land
void HttpErrorString(const FunctionCallbackInfo<Value>& args) {
  Environment* env = Environment::GetCurrent(args);
  uint32_t val = args[0]->Uint32Value(env->context()).ToChecked();
  args.GetReturnValue().Set(
      String::NewFromOneByte(
          env->isolate(),
          reinterpret_cast<const uint8_t*>(nghttp2_strerror(val)),
          NewStringType::kInternalized).ToLocalChecked());
}


// Serializes the settings object into a Buffer instance that
// would be suitable, for instance, for creating the Base64
// output for an HTTP2-Settings header field.
void PackSettings(const FunctionCallbackInfo<Value>& args) {
  Environment* env = Environment::GetCurrent(args);
  // TODO(addaleax): We should not be creating a full AsyncWrap for this.
  Local<Object> obj;
  if (!env->http2settings_constructor_template()
           ->NewInstance(env->context())
           .ToLocal(&obj)) {
    return;
  }
  Http2Session::Http2Settings settings(env, nullptr, obj);
  args.GetReturnValue().Set(settings.Pack());
}

// A TypedArray instance is shared between C++ and JS land to contain the
// default SETTINGS. RefreshDefaultSettings updates that TypedArray with the
// default values.
void RefreshDefaultSettings(const FunctionCallbackInfo<Value>& args) {
  Environment* env = Environment::GetCurrent(args);
  Http2Session::Http2Settings::RefreshDefaults(env);
}

// Sets the next stream ID the Http2Session. If successful, returns true.
void Http2Session::SetNextStreamID(const FunctionCallbackInfo<Value>& args) {
  Environment* env = Environment::GetCurrent(args);
  Http2Session* session;
  ASSIGN_OR_RETURN_UNWRAP(&session, args.Holder());
  int32_t id = args[0]->Int32Value(env->context()).ToChecked();
  if (nghttp2_session_set_next_stream_id(**session, id) < 0) {
    Debug(session, "failed to set next stream id to %d", id);
    return args.GetReturnValue().Set(false);
  }
  args.GetReturnValue().Set(true);
  Debug(session, "set next stream id to %d", id);
}

// A TypedArray instance is shared between C++ and JS land to contain the
// SETTINGS (either remote or local). RefreshSettings updates the current
// values established for each of the settings so those can be read in JS land.
template <get_setting fn>
void Http2Session::RefreshSettings(const FunctionCallbackInfo<Value>& args) {
  Environment* env = Environment::GetCurrent(args);
  Http2Session* session;
  ASSIGN_OR_RETURN_UNWRAP(&session, args.Holder());
  Http2Settings::Update(env, session, fn);
  Debug(session, "settings refreshed for session");
}

// A TypedArray instance is shared between C++ and JS land to contain state
// information of the current Http2Session. This updates the values in the
// TypedArray so those can be read in JS land.
void Http2Session::RefreshState(const FunctionCallbackInfo<Value>& args) {
  Environment* env = Environment::GetCurrent(args);
  Http2Session* session;
  ASSIGN_OR_RETURN_UNWRAP(&session, args.Holder());
  Debug(session, "refreshing state");

  AliasedFloat64Array& buffer = env->http2_state()->session_state_buffer;

  nghttp2_session* s = **session;

  buffer[IDX_SESSION_STATE_EFFECTIVE_LOCAL_WINDOW_SIZE] =
      nghttp2_session_get_effective_local_window_size(s);
  buffer[IDX_SESSION_STATE_EFFECTIVE_RECV_DATA_LENGTH] =
      nghttp2_session_get_effective_recv_data_length(s);
  buffer[IDX_SESSION_STATE_NEXT_STREAM_ID] =
      nghttp2_session_get_next_stream_id(s);
  buffer[IDX_SESSION_STATE_LOCAL_WINDOW_SIZE] =
      nghttp2_session_get_local_window_size(s);
  buffer[IDX_SESSION_STATE_LAST_PROC_STREAM_ID] =
      nghttp2_session_get_last_proc_stream_id(s);
  buffer[IDX_SESSION_STATE_REMOTE_WINDOW_SIZE] =
      nghttp2_session_get_remote_window_size(s);
  buffer[IDX_SESSION_STATE_OUTBOUND_QUEUE_SIZE] =
      nghttp2_session_get_outbound_queue_size(s);
  buffer[IDX_SESSION_STATE_HD_DEFLATE_DYNAMIC_TABLE_SIZE] =
      nghttp2_session_get_hd_deflate_dynamic_table_size(s);
  buffer[IDX_SESSION_STATE_HD_INFLATE_DYNAMIC_TABLE_SIZE] =
      nghttp2_session_get_hd_inflate_dynamic_table_size(s);
}


// Constructor for new Http2Session instances.
void Http2Session::New(const FunctionCallbackInfo<Value>& args) {
  Environment* env = Environment::GetCurrent(args);
  CHECK(args.IsConstructCall());
  int val = args[0]->IntegerValue(env->context()).ToChecked();
  nghttp2_session_type type = static_cast<nghttp2_session_type>(val);
  Http2Session* session = new Http2Session(env, args.This(), type);
  session->get_async_id();  // avoid compiler warning
  Debug(session, "session created");
}


// Binds the Http2Session with a StreamBase used for i/o
void Http2Session::Consume(const FunctionCallbackInfo<Value>& args) {
  Http2Session* session;
  ASSIGN_OR_RETURN_UNWRAP(&session, args.Holder());
  CHECK(args[0]->IsObject());
  session->Consume(args[0].As<Object>());
}

// Destroys the Http2Session instance and renders it unusable
void Http2Session::Destroy(const FunctionCallbackInfo<Value>& args) {
  Http2Session* session;
  ASSIGN_OR_RETURN_UNWRAP(&session, args.Holder());
  Debug(session, "destroying session");
  Environment* env = Environment::GetCurrent(args);
  Local<Context> context = env->context();

  uint32_t code = args[0]->Uint32Value(context).ToChecked();
  bool socketDestroyed = args[1]->BooleanValue(env->isolate());

  session->Close(code, socketDestroyed);
}

// Submits a new request on the Http2Session and returns either an error code
// or the Http2Stream object.
void Http2Session::Request(const FunctionCallbackInfo<Value>& args) {
  Http2Session* session;
  ASSIGN_OR_RETURN_UNWRAP(&session, args.Holder());
  Environment* env = session->env();
  Local<Context> context = env->context();
  Isolate* isolate = env->isolate();

  Local<Array> headers = args[0].As<Array>();
  int options = args[1]->IntegerValue(context).ToChecked();
  Http2Priority priority(env, args[2], args[3], args[4]);

  Headers list(isolate, context, headers);

  Debug(session, "request submitted");

  int32_t ret = 0;
  Http2Stream* stream =
      session->Http2Session::SubmitRequest(*priority, *list, list.length(),
                                           &ret, options);

  if (ret <= 0 || stream == nullptr) {
    Debug(session, "could not submit request: %s", nghttp2_strerror(ret));
    return args.GetReturnValue().Set(ret);
  }

  Debug(session, "request submitted, new stream id %d", stream->id());
  args.GetReturnValue().Set(stream->object());
}

// Submits a GOAWAY frame to signal that the Http2Session is in the process
// of shutting down. Note that this function does not actually alter the
// state of the Http2Session, it's simply a notification.
void Http2Session::Goaway(uint32_t code,
                          int32_t lastStreamID,
                          const uint8_t* data,
                          size_t len) {
  if (IsDestroyed())
    return;

  Http2Scope h2scope(this);
  // the last proc stream id is the most recently created Http2Stream.
  if (lastStreamID <= 0)
    lastStreamID = nghttp2_session_get_last_proc_stream_id(session_);
  Debug(this, "submitting goaway");
  nghttp2_submit_goaway(session_, NGHTTP2_FLAG_NONE,
                        lastStreamID, code, data, len);
}

// Submits a GOAWAY frame to signal that the Http2Session is in the process
// of shutting down. The opaque data argument is an optional TypedArray that
// can be used to send debugging data to the connected peer.
void Http2Session::Goaway(const FunctionCallbackInfo<Value>& args) {
  Environment* env = Environment::GetCurrent(args);
  Local<Context> context = env->context();
  Http2Session* session;
  ASSIGN_OR_RETURN_UNWRAP(&session, args.Holder());

  uint32_t code = args[0]->Uint32Value(context).ToChecked();
  int32_t lastStreamID = args[1]->Int32Value(context).ToChecked();
  ArrayBufferViewContents<uint8_t> opaque_data;

  if (args[2]->IsArrayBufferView()) {
    opaque_data.Read(args[2].As<ArrayBufferView>());
  }

  session->Goaway(code, lastStreamID, opaque_data.data(), opaque_data.length());
}

// Update accounting of data chunks. This is used primarily to manage timeout
// logic when using the FD Provider.
void Http2Session::UpdateChunksSent(const FunctionCallbackInfo<Value>& args) {
  Environment* env = Environment::GetCurrent(args);
  Isolate* isolate = env->isolate();
  HandleScope scope(isolate);
  Http2Session* session;
  ASSIGN_OR_RETURN_UNWRAP(&session, args.Holder());

  uint32_t length = session->chunks_sent_since_last_write_;

  session->object()->Set(env->context(),
                         env->chunks_sent_since_last_write_string(),
                         Integer::NewFromUnsigned(isolate, length)).Check();

  args.GetReturnValue().Set(length);
}

// Submits an RST_STREAM frame effectively closing the Http2Stream. Note that
// this *WILL* alter the state of the stream, causing the OnStreamClose
// callback to the triggered.
void Http2Stream::RstStream(const FunctionCallbackInfo<Value>& args) {
  Environment* env = Environment::GetCurrent(args);
  Local<Context> context = env->context();
  Http2Stream* stream;
  ASSIGN_OR_RETURN_UNWRAP(&stream, args.Holder());
  uint32_t code = args[0]->Uint32Value(context).ToChecked();
  Debug(stream, "sending rst_stream with code %d", code);
  stream->SubmitRstStream(code);
}

// Initiates a response on the Http2Stream using the StreamBase API to provide
// outbound DATA frames.
void Http2Stream::Respond(const FunctionCallbackInfo<Value>& args) {
  Environment* env = Environment::GetCurrent(args);
  Local<Context> context = env->context();
  Isolate* isolate = env->isolate();
  Http2Stream* stream;
  ASSIGN_OR_RETURN_UNWRAP(&stream, args.Holder());

  Local<Array> headers = args[0].As<Array>();
  int options = args[1]->IntegerValue(context).ToChecked();

  Headers list(isolate, context, headers);

  args.GetReturnValue().Set(
      stream->SubmitResponse(*list, list.length(), options));
  Debug(stream, "response submitted");
}


// Submits informational headers on the Http2Stream
void Http2Stream::Info(const FunctionCallbackInfo<Value>& args) {
  Environment* env = Environment::GetCurrent(args);
  Local<Context> context = env->context();
  Isolate* isolate = env->isolate();
  Http2Stream* stream;
  ASSIGN_OR_RETURN_UNWRAP(&stream, args.Holder());

  Local<Array> headers = args[0].As<Array>();

  Headers list(isolate, context, headers);
  args.GetReturnValue().Set(stream->SubmitInfo(*list, list.length()));
  Debug(stream, "%d informational headers sent", list.length());
}

// Submits trailing headers on the Http2Stream
void Http2Stream::Trailers(const FunctionCallbackInfo<Value>& args) {
  Environment* env = Environment::GetCurrent(args);
  Local<Context> context = env->context();
  Isolate* isolate = env->isolate();
  Http2Stream* stream;
  ASSIGN_OR_RETURN_UNWRAP(&stream, args.Holder());

  Local<Array> headers = args[0].As<Array>();

  Headers list(isolate, context, headers);
  args.GetReturnValue().Set(stream->SubmitTrailers(*list, list.length()));
  Debug(stream, "%d trailing headers sent", list.length());
}

// Grab the numeric id of the Http2Stream
void Http2Stream::GetID(const FunctionCallbackInfo<Value>& args) {
  Http2Stream* stream;
  ASSIGN_OR_RETURN_UNWRAP(&stream, args.Holder());
  args.GetReturnValue().Set(stream->id());
}

// Destroy the Http2Stream, rendering it no longer usable
void Http2Stream::Destroy(const FunctionCallbackInfo<Value>& args) {
  Http2Stream* stream;
  ASSIGN_OR_RETURN_UNWRAP(&stream, args.Holder());
  Debug(stream, "destroying stream");
  stream->Destroy();
}

// Prompt the Http2Stream to begin sending data to the JS land.
void Http2Stream::FlushData(const FunctionCallbackInfo<Value>& args) {
  Http2Stream* stream;
  ASSIGN_OR_RETURN_UNWRAP(&stream, args.Holder());
  stream->ReadStart();
  Debug(stream, "data flushed to js");
}

// Initiate a Push Promise and create the associated Http2Stream
void Http2Stream::PushPromise(const FunctionCallbackInfo<Value>& args) {
  Environment* env = Environment::GetCurrent(args);
  Local<Context> context = env->context();
  Isolate* isolate = env->isolate();
  Http2Stream* parent;
  ASSIGN_OR_RETURN_UNWRAP(&parent, args.Holder());

  Local<Array> headers = args[0].As<Array>();
  int options = args[1]->IntegerValue(context).ToChecked();

  Headers list(isolate, context, headers);

  Debug(parent, "creating push promise");

  int32_t ret = 0;
  Http2Stream* stream = parent->SubmitPushPromise(*list, list.length(),
                                                  &ret, options);
  if (ret <= 0 || stream == nullptr) {
    Debug(parent, "failed to create push stream: %d", ret);
    return args.GetReturnValue().Set(ret);
  }
  Debug(parent, "push stream %d created", stream->id());
  args.GetReturnValue().Set(stream->object());
}

// Send a PRIORITY frame
void Http2Stream::Priority(const FunctionCallbackInfo<Value>& args) {
  Environment* env = Environment::GetCurrent(args);
  Http2Stream* stream;
  ASSIGN_OR_RETURN_UNWRAP(&stream, args.Holder());

  Http2Priority priority(env, args[0], args[1], args[2]);
  bool silent = args[3]->BooleanValue(env->isolate());

  CHECK_EQ(stream->SubmitPriority(*priority, silent), 0);
  Debug(stream, "priority submitted");
}

// A TypedArray shared by C++ and JS land is used to communicate state
// information about the Http2Stream. This updates the values in that
// TypedArray so that the state can be read by JS.
void Http2Stream::RefreshState(const FunctionCallbackInfo<Value>& args) {
  Environment* env = Environment::GetCurrent(args);
  Http2Stream* stream;
  ASSIGN_OR_RETURN_UNWRAP(&stream, args.Holder());

  Debug(stream, "refreshing state");

  AliasedFloat64Array& buffer = env->http2_state()->stream_state_buffer;

  nghttp2_stream* str = **stream;
  nghttp2_session* s = **(stream->session());

  if (str == nullptr) {
    buffer[IDX_STREAM_STATE] = NGHTTP2_STREAM_STATE_IDLE;
    buffer[IDX_STREAM_STATE_WEIGHT] =
        buffer[IDX_STREAM_STATE_SUM_DEPENDENCY_WEIGHT] =
        buffer[IDX_STREAM_STATE_LOCAL_CLOSE] =
        buffer[IDX_STREAM_STATE_REMOTE_CLOSE] =
        buffer[IDX_STREAM_STATE_LOCAL_WINDOW_SIZE] = 0;
  } else {
    buffer[IDX_STREAM_STATE] =
        nghttp2_stream_get_state(str);
    buffer[IDX_STREAM_STATE_WEIGHT] =
        nghttp2_stream_get_weight(str);
    buffer[IDX_STREAM_STATE_SUM_DEPENDENCY_WEIGHT] =
        nghttp2_stream_get_sum_dependency_weight(str);
    buffer[IDX_STREAM_STATE_LOCAL_CLOSE] =
        nghttp2_session_get_stream_local_close(s, stream->id());
    buffer[IDX_STREAM_STATE_REMOTE_CLOSE] =
        nghttp2_session_get_stream_remote_close(s, stream->id());
    buffer[IDX_STREAM_STATE_LOCAL_WINDOW_SIZE] =
        nghttp2_session_get_stream_local_window_size(s, stream->id());
  }
}

void Http2Session::AltSvc(int32_t id,
                          uint8_t* origin,
                          size_t origin_len,
                          uint8_t* value,
                          size_t value_len) {
  Http2Scope h2scope(this);
  CHECK_EQ(nghttp2_submit_altsvc(session_, NGHTTP2_FLAG_NONE, id,
                                 origin, origin_len, value, value_len), 0);
}

void Http2Session::Origin(nghttp2_origin_entry* ov, size_t count) {
  Http2Scope h2scope(this);
  CHECK_EQ(nghttp2_submit_origin(session_, NGHTTP2_FLAG_NONE, ov, count), 0);
}

// Submits an AltSvc frame to be sent to the connected peer.
void Http2Session::AltSvc(const FunctionCallbackInfo<Value>& args) {
  Environment* env = Environment::GetCurrent(args);
  Http2Session* session;
  ASSIGN_OR_RETURN_UNWRAP(&session, args.Holder());

  int32_t id = args[0]->Int32Value(env->context()).ToChecked();

  // origin and value are both required to be ASCII, handle them as such.
  Local<String> origin_str = args[1]->ToString(env->context()).ToLocalChecked();
  Local<String> value_str = args[2]->ToString(env->context()).ToLocalChecked();

  size_t origin_len = origin_str->Length();
  size_t value_len = value_str->Length();

  CHECK_LE(origin_len + value_len, 16382);  // Max permitted for ALTSVC
  // Verify that origin len != 0 if stream id == 0, or
  // that origin len == 0 if stream id != 0
  CHECK((origin_len != 0 && id == 0) || (origin_len == 0 && id != 0));

  MaybeStackBuffer<uint8_t> origin(origin_len);
  MaybeStackBuffer<uint8_t> value(value_len);
  origin_str->WriteOneByte(env->isolate(), *origin);
  value_str->WriteOneByte(env->isolate(), *value);

  session->AltSvc(id, *origin, origin_len, *value, value_len);
}

void Http2Session::Origin(const FunctionCallbackInfo<Value>& args) {
  Environment* env = Environment::GetCurrent(args);
  Local<Context> context = env->context();
  Http2Session* session;
  ASSIGN_OR_RETURN_UNWRAP(&session, args.Holder());

  Local<String> origin_string = args[0].As<String>();
  int count = args[1]->IntegerValue(context).ToChecked();


  Origins origins(env->isolate(),
                  env->context(),
                  origin_string,
                  count);

  session->Origin(*origins, origins.length());
}

// Submits a PING frame to be sent to the connected peer.
void Http2Session::Ping(const FunctionCallbackInfo<Value>& args) {
  Environment* env = Environment::GetCurrent(args);
  Http2Session* session;
  ASSIGN_OR_RETURN_UNWRAP(&session, args.Holder());

  // A PING frame may have exactly 8 bytes of payload data. If not provided,
  // then the current hrtime will be used as the payload.
  ArrayBufferViewContents<uint8_t, 8> payload;
  if (args[0]->IsArrayBufferView()) {
    payload.Read(args[0].As<ArrayBufferView>());
    CHECK_EQ(payload.length(), 8);
  }

  Local<Object> obj;
  if (!env->http2ping_constructor_template()
           ->NewInstance(env->context())
           .ToLocal(&obj)) {
    return;
  }
  if (obj->Set(env->context(), env->ondone_string(), args[1]).IsNothing())
    return;

  Http2Ping* ping = session->AddPing(std::make_unique<Http2Ping>(session, obj));
  // To prevent abuse, we strictly limit the number of unacknowledged PING
  // frames that may be sent at any given time. This is configurable in the
  // Options when creating a Http2Session.
  if (ping == nullptr) return args.GetReturnValue().Set(false);

  // The Ping itself is an Async resource. When the acknowledgement is received,
  // the callback will be invoked and a notification sent out to JS land. The
  // notification will include the duration of the ping, allowing the round
  // trip to be measured.
  ping->Send(payload.data());
  args.GetReturnValue().Set(true);
}

// Submits a SETTINGS frame for the Http2Session
void Http2Session::Settings(const FunctionCallbackInfo<Value>& args) {
  Environment* env = Environment::GetCurrent(args);
  Http2Session* session;
  ASSIGN_OR_RETURN_UNWRAP(&session, args.Holder());

  Local<Object> obj;
  if (!env->http2settings_constructor_template()
           ->NewInstance(env->context())
           .ToLocal(&obj)) {
    return;
  }
  if (obj->Set(env->context(), env->ondone_string(), args[0]).IsNothing())
    return;

  Http2Session::Http2Settings* settings = session->AddSettings(
      std::make_unique<Http2Settings>(session->env(), session, obj, 0));
  if (settings == nullptr) return args.GetReturnValue().Set(false);

  settings->Send();
  args.GetReturnValue().Set(true);
}

std::unique_ptr<Http2Session::Http2Ping> Http2Session::PopPing() {
  std::unique_ptr<Http2Ping> ping;
  if (!outstanding_pings_.empty()) {
    ping = std::move(outstanding_pings_.front());
    outstanding_pings_.pop();
    DecrementCurrentSessionMemory(sizeof(*ping));
  }
  return ping;
}

Http2Session::Http2Ping* Http2Session::AddPing(
    std::unique_ptr<Http2Session::Http2Ping> ping) {
  if (outstanding_pings_.size() == max_outstanding_pings_) {
    ping->Done(false);
    return nullptr;
  }
  Http2Ping* ptr = ping.get();
  outstanding_pings_.emplace(std::move(ping));
  IncrementCurrentSessionMemory(sizeof(*ping));
  return ptr;
}

std::unique_ptr<Http2Session::Http2Settings> Http2Session::PopSettings() {
  std::unique_ptr<Http2Settings> settings;
  if (!outstanding_settings_.empty()) {
    settings = std::move(outstanding_settings_.front());
    outstanding_settings_.pop();
    DecrementCurrentSessionMemory(sizeof(*settings));
  }
  return settings;
}

Http2Session::Http2Settings* Http2Session::AddSettings(
    std::unique_ptr<Http2Session::Http2Settings> settings) {
  if (outstanding_settings_.size() == max_outstanding_settings_) {
    settings->Done(false);
    return nullptr;
  }
  Http2Settings* ptr = settings.get();
  outstanding_settings_.emplace(std::move(settings));
  IncrementCurrentSessionMemory(sizeof(*settings));
  return ptr;
}

Http2Session::Http2Ping::Http2Ping(Http2Session* session, Local<Object> obj)
    : AsyncWrap(session->env(), obj, AsyncWrap::PROVIDER_HTTP2PING),
      session_(session),
      startTime_(uv_hrtime()) {
  RemoveCleanupHook();  // This object is owned by the Http2Session.
}

void Http2Session::Http2Ping::Send(const uint8_t* payload) {
  CHECK_NOT_NULL(session_);
  uint8_t data[8];
  if (payload == nullptr) {
    memcpy(&data, &startTime_, arraysize(data));
    payload = data;
  }
  Http2Scope h2scope(session_);
  CHECK_EQ(nghttp2_submit_ping(**session_, NGHTTP2_FLAG_NONE, payload), 0);
}

void Http2Session::Http2Ping::Done(bool ack, const uint8_t* payload) {
  uint64_t duration_ns = uv_hrtime() - startTime_;
  double duration_ms = duration_ns / 1e6;
  if (session_ != nullptr) session_->statistics_.ping_rtt = duration_ns;

  HandleScope handle_scope(env()->isolate());
  Context::Scope context_scope(env()->context());

  Local<Value> buf = Undefined(env()->isolate());
  if (payload != nullptr) {
    buf = Buffer::Copy(env()->isolate(),
                       reinterpret_cast<const char*>(payload),
                       8).ToLocalChecked();
  }

  Local<Value> argv[] = {
    Boolean::New(env()->isolate(), ack),
    Number::New(env()->isolate(), duration_ms),
    buf
  };
  MakeCallback(env()->ondone_string(), arraysize(argv), argv);
}

void Http2Session::Http2Ping::DetachFromSession() {
  session_ = nullptr;
}

void nghttp2_stream_write::MemoryInfo(MemoryTracker* tracker) const {
  if (req_wrap != nullptr)
    tracker->TrackField("req_wrap", req_wrap->GetAsyncWrap());
  tracker->TrackField("buf", buf);
}


void nghttp2_header::MemoryInfo(MemoryTracker* tracker) const {
  tracker->TrackFieldWithSize("name", nghttp2_rcbuf_get_buf(name).len);
  tracker->TrackFieldWithSize("value", nghttp2_rcbuf_get_buf(value).len);
}

void SetCallbackFunctions(const FunctionCallbackInfo<Value>& args) {
  Environment* env = Environment::GetCurrent(args);
  CHECK_EQ(args.Length(), 12);

#define SET_FUNCTION(arg, name)                                               \
  CHECK(args[arg]->IsFunction());                                             \
  env->set_http2session_on_ ## name ## _function(args[arg].As<Function>());

  SET_FUNCTION(0, error)
  SET_FUNCTION(1, priority)
  SET_FUNCTION(2, settings)
  SET_FUNCTION(3, ping)
  SET_FUNCTION(4, headers)
  SET_FUNCTION(5, frame_error)
  SET_FUNCTION(6, goaway_data)
  SET_FUNCTION(7, altsvc)
  SET_FUNCTION(8, origin)
  SET_FUNCTION(9, select_padding)
  SET_FUNCTION(10, stream_trailers)
  SET_FUNCTION(11, stream_close)

#undef SET_FUNCTION
}

// Set up the process.binding('http2') binding.
void Initialize(Local<Object> target,
                Local<Value> unused,
                Local<Context> context,
                void* priv) {
  Environment* env = Environment::GetCurrent(context);
  Isolate* isolate = env->isolate();
  HandleScope scope(isolate);

  std::unique_ptr<Http2State> state(new Http2State(isolate));

#define SET_STATE_TYPEDARRAY(name, field)             \
  target->Set(context,                                \
              FIXED_ONE_BYTE_STRING(isolate, (name)), \
              (field)).FromJust()

  // Initialize the buffer used for padding callbacks
  SET_STATE_TYPEDARRAY(
    "paddingBuffer", state->padding_buffer.GetJSArray());
  // Initialize the buffer used to store the session state
  SET_STATE_TYPEDARRAY(
    "sessionState", state->session_state_buffer.GetJSArray());
  // Initialize the buffer used to store the stream state
  SET_STATE_TYPEDARRAY(
    "streamState", state->stream_state_buffer.GetJSArray());
  SET_STATE_TYPEDARRAY(
    "settingsBuffer", state->settings_buffer.GetJSArray());
  SET_STATE_TYPEDARRAY(
    "optionsBuffer", state->options_buffer.GetJSArray());
  SET_STATE_TYPEDARRAY(
    "streamStats", state->stream_stats_buffer.GetJSArray());
  SET_STATE_TYPEDARRAY(
    "sessionStats", state->session_stats_buffer.GetJSArray());
#undef SET_STATE_TYPEDARRAY

  env->set_http2_state(std::move(state));

  NODE_DEFINE_CONSTANT(target, PADDING_BUF_FRAME_LENGTH);
  NODE_DEFINE_CONSTANT(target, PADDING_BUF_MAX_PAYLOAD_LENGTH);
  NODE_DEFINE_CONSTANT(target, PADDING_BUF_RETURN_VALUE);

  NODE_DEFINE_CONSTANT(target, kBitfield);
  NODE_DEFINE_CONSTANT(target, kSessionPriorityListenerCount);
  NODE_DEFINE_CONSTANT(target, kSessionFrameErrorListenerCount);
  NODE_DEFINE_CONSTANT(target, kSessionUint8FieldCount);

  NODE_DEFINE_CONSTANT(target, kSessionHasRemoteSettingsListeners);
  NODE_DEFINE_CONSTANT(target, kSessionRemoteSettingsIsUpToDate);
  NODE_DEFINE_CONSTANT(target, kSessionHasPingListeners);
  NODE_DEFINE_CONSTANT(target, kSessionHasAltsvcListeners);

  // Method to fetch the nghttp2 string description of an nghttp2 error code
  env->SetMethod(target, "nghttp2ErrorString", HttpErrorString);

  Local<String> http2SessionClassName =
    FIXED_ONE_BYTE_STRING(isolate, "Http2Session");

  Local<FunctionTemplate> ping = FunctionTemplate::New(env->isolate());
  ping->SetClassName(FIXED_ONE_BYTE_STRING(env->isolate(), "Http2Ping"));
  ping->Inherit(AsyncWrap::GetConstructorTemplate(env));
  Local<ObjectTemplate> pingt = ping->InstanceTemplate();
  pingt->SetInternalFieldCount(1);
  env->set_http2ping_constructor_template(pingt);

  Local<FunctionTemplate> setting = FunctionTemplate::New(env->isolate());
  setting->SetClassName(FIXED_ONE_BYTE_STRING(env->isolate(), "Http2Setting"));
  setting->Inherit(AsyncWrap::GetConstructorTemplate(env));
  Local<ObjectTemplate> settingt = setting->InstanceTemplate();
  settingt->SetInternalFieldCount(1);
  env->set_http2settings_constructor_template(settingt);

  Local<FunctionTemplate> stream = FunctionTemplate::New(env->isolate());
  stream->SetClassName(FIXED_ONE_BYTE_STRING(env->isolate(), "Http2Stream"));
  env->SetProtoMethod(stream, "id", Http2Stream::GetID);
  env->SetProtoMethod(stream, "destroy", Http2Stream::Destroy);
  env->SetProtoMethod(stream, "flushData", Http2Stream::FlushData);
  env->SetProtoMethod(stream, "priority", Http2Stream::Priority);
  env->SetProtoMethod(stream, "pushPromise", Http2Stream::PushPromise);
  env->SetProtoMethod(stream, "info", Http2Stream::Info);
  env->SetProtoMethod(stream, "trailers", Http2Stream::Trailers);
  env->SetProtoMethod(stream, "respond", Http2Stream::Respond);
  env->SetProtoMethod(stream, "rstStream", Http2Stream::RstStream);
  env->SetProtoMethod(stream, "refreshState", Http2Stream::RefreshState);
  stream->Inherit(AsyncWrap::GetConstructorTemplate(env));
  StreamBase::AddMethods(env, stream);
  Local<ObjectTemplate> streamt = stream->InstanceTemplate();
  streamt->SetInternalFieldCount(StreamBase::kStreamBaseFieldCount);
  env->set_http2stream_constructor_template(streamt);
  target->Set(context,
              FIXED_ONE_BYTE_STRING(env->isolate(), "Http2Stream"),
              stream->GetFunction(env->context()).ToLocalChecked()).Check();

  Local<FunctionTemplate> session =
      env->NewFunctionTemplate(Http2Session::New);
  session->SetClassName(http2SessionClassName);
  session->InstanceTemplate()->SetInternalFieldCount(1);
  session->Inherit(AsyncWrap::GetConstructorTemplate(env));
  env->SetProtoMethod(session, "origin", Http2Session::Origin);
  env->SetProtoMethod(session, "altsvc", Http2Session::AltSvc);
  env->SetProtoMethod(session, "ping", Http2Session::Ping);
  env->SetProtoMethod(session, "consume", Http2Session::Consume);
  env->SetProtoMethod(session, "destroy", Http2Session::Destroy);
  env->SetProtoMethod(session, "goaway", Http2Session::Goaway);
  env->SetProtoMethod(session, "settings", Http2Session::Settings);
  env->SetProtoMethod(session, "request", Http2Session::Request);
  env->SetProtoMethod(session, "setNextStreamID",
                      Http2Session::SetNextStreamID);
  env->SetProtoMethod(session, "updateChunksSent",
                      Http2Session::UpdateChunksSent);
  env->SetProtoMethod(session, "refreshState", Http2Session::RefreshState);
  env->SetProtoMethod(
      session, "localSettings",
      Http2Session::RefreshSettings<nghttp2_session_get_local_settings>);
  env->SetProtoMethod(
      session, "remoteSettings",
      Http2Session::RefreshSettings<nghttp2_session_get_remote_settings>);
  target->Set(context,
              http2SessionClassName,
              session->GetFunction(env->context()).ToLocalChecked()).Check();

  Local<Object> constants = Object::New(isolate);
  Local<Array> name_for_error_code = Array::New(isolate);

#define NODE_NGHTTP2_ERROR_CODES(V)                       \
  V(NGHTTP2_SESSION_SERVER);                              \
  V(NGHTTP2_SESSION_CLIENT);                              \
  V(NGHTTP2_STREAM_STATE_IDLE);                           \
  V(NGHTTP2_STREAM_STATE_OPEN);                           \
  V(NGHTTP2_STREAM_STATE_RESERVED_LOCAL);                 \
  V(NGHTTP2_STREAM_STATE_RESERVED_REMOTE);                \
  V(NGHTTP2_STREAM_STATE_HALF_CLOSED_LOCAL);              \
  V(NGHTTP2_STREAM_STATE_HALF_CLOSED_REMOTE);             \
  V(NGHTTP2_STREAM_STATE_CLOSED);                         \
  V(NGHTTP2_NO_ERROR);                                    \
  V(NGHTTP2_PROTOCOL_ERROR);                              \
  V(NGHTTP2_INTERNAL_ERROR);                              \
  V(NGHTTP2_FLOW_CONTROL_ERROR);                          \
  V(NGHTTP2_SETTINGS_TIMEOUT);                            \
  V(NGHTTP2_STREAM_CLOSED);                               \
  V(NGHTTP2_FRAME_SIZE_ERROR);                            \
  V(NGHTTP2_REFUSED_STREAM);                              \
  V(NGHTTP2_CANCEL);                                      \
  V(NGHTTP2_COMPRESSION_ERROR);                           \
  V(NGHTTP2_CONNECT_ERROR);                               \
  V(NGHTTP2_ENHANCE_YOUR_CALM);                           \
  V(NGHTTP2_INADEQUATE_SECURITY);                         \
  V(NGHTTP2_HTTP_1_1_REQUIRED);                           \

#define V(name)                                                         \
  NODE_DEFINE_CONSTANT(constants, name);                                \
  name_for_error_code->Set(env->context(),                              \
                           static_cast<int>(name),                      \
                           FIXED_ONE_BYTE_STRING(isolate,               \
                                                 #name)).Check();
  NODE_NGHTTP2_ERROR_CODES(V)
#undef V

  NODE_DEFINE_HIDDEN_CONSTANT(constants, NGHTTP2_HCAT_REQUEST);
  NODE_DEFINE_HIDDEN_CONSTANT(constants, NGHTTP2_HCAT_RESPONSE);
  NODE_DEFINE_HIDDEN_CONSTANT(constants, NGHTTP2_HCAT_PUSH_RESPONSE);
  NODE_DEFINE_HIDDEN_CONSTANT(constants, NGHTTP2_HCAT_HEADERS);
  NODE_DEFINE_HIDDEN_CONSTANT(constants, NGHTTP2_NV_FLAG_NONE);
  NODE_DEFINE_HIDDEN_CONSTANT(constants, NGHTTP2_NV_FLAG_NO_INDEX);
  NODE_DEFINE_HIDDEN_CONSTANT(constants, NGHTTP2_ERR_DEFERRED);
  NODE_DEFINE_HIDDEN_CONSTANT(constants, NGHTTP2_ERR_STREAM_ID_NOT_AVAILABLE);
  NODE_DEFINE_HIDDEN_CONSTANT(constants, NGHTTP2_ERR_INVALID_ARGUMENT);
  NODE_DEFINE_HIDDEN_CONSTANT(constants, NGHTTP2_ERR_STREAM_CLOSED);
  NODE_DEFINE_CONSTANT(constants, NGHTTP2_ERR_FRAME_SIZE_ERROR);

  NODE_DEFINE_HIDDEN_CONSTANT(constants, STREAM_OPTION_EMPTY_PAYLOAD);
  NODE_DEFINE_HIDDEN_CONSTANT(constants, STREAM_OPTION_GET_TRAILERS);

  NODE_DEFINE_CONSTANT(constants, NGHTTP2_FLAG_NONE);
  NODE_DEFINE_CONSTANT(constants, NGHTTP2_FLAG_END_STREAM);
  NODE_DEFINE_CONSTANT(constants, NGHTTP2_FLAG_END_HEADERS);
  NODE_DEFINE_CONSTANT(constants, NGHTTP2_FLAG_ACK);
  NODE_DEFINE_CONSTANT(constants, NGHTTP2_FLAG_PADDED);
  NODE_DEFINE_CONSTANT(constants, NGHTTP2_FLAG_PRIORITY);

  NODE_DEFINE_CONSTANT(constants, DEFAULT_SETTINGS_HEADER_TABLE_SIZE);
  NODE_DEFINE_CONSTANT(constants, DEFAULT_SETTINGS_ENABLE_PUSH);
  NODE_DEFINE_CONSTANT(constants, DEFAULT_SETTINGS_INITIAL_WINDOW_SIZE);
  NODE_DEFINE_CONSTANT(constants, DEFAULT_SETTINGS_MAX_FRAME_SIZE);
  NODE_DEFINE_CONSTANT(constants, MAX_MAX_FRAME_SIZE);
  NODE_DEFINE_CONSTANT(constants, MIN_MAX_FRAME_SIZE);
  NODE_DEFINE_CONSTANT(constants, MAX_INITIAL_WINDOW_SIZE);
  NODE_DEFINE_CONSTANT(constants, NGHTTP2_DEFAULT_WEIGHT);

  NODE_DEFINE_CONSTANT(constants, NGHTTP2_SETTINGS_HEADER_TABLE_SIZE);
  NODE_DEFINE_CONSTANT(constants, NGHTTP2_SETTINGS_ENABLE_PUSH);
  NODE_DEFINE_CONSTANT(constants, NGHTTP2_SETTINGS_MAX_CONCURRENT_STREAMS);
  NODE_DEFINE_CONSTANT(constants, NGHTTP2_SETTINGS_INITIAL_WINDOW_SIZE);
  NODE_DEFINE_CONSTANT(constants, NGHTTP2_SETTINGS_MAX_FRAME_SIZE);
  NODE_DEFINE_CONSTANT(constants, NGHTTP2_SETTINGS_MAX_HEADER_LIST_SIZE);
  NODE_DEFINE_CONSTANT(constants, NGHTTP2_SETTINGS_ENABLE_CONNECT_PROTOCOL);

  NODE_DEFINE_CONSTANT(constants, PADDING_STRATEGY_NONE);
  NODE_DEFINE_CONSTANT(constants, PADDING_STRATEGY_ALIGNED);
  NODE_DEFINE_CONSTANT(constants, PADDING_STRATEGY_MAX);
  NODE_DEFINE_CONSTANT(constants, PADDING_STRATEGY_CALLBACK);

#define STRING_CONSTANT(NAME, VALUE)                                          \
  NODE_DEFINE_STRING_CONSTANT(constants, "HTTP2_HEADER_" # NAME, VALUE);
HTTP_KNOWN_HEADERS(STRING_CONSTANT)
#undef STRING_CONSTANT

#define STRING_CONSTANT(NAME, VALUE)                                          \
  NODE_DEFINE_STRING_CONSTANT(constants, "HTTP2_METHOD_" # NAME, VALUE);
HTTP_KNOWN_METHODS(STRING_CONSTANT)
#undef STRING_CONSTANT

#define V(name, _) NODE_DEFINE_CONSTANT(constants, HTTP_STATUS_##name);
HTTP_STATUS_CODES(V)
#undef V

  env->SetMethod(target, "refreshDefaultSettings", RefreshDefaultSettings);
  env->SetMethod(target, "packSettings", PackSettings);
  env->SetMethod(target, "setCallbackFunctions", SetCallbackFunctions);

  target->Set(context,
              env->constants_string(),
              constants).Check();
  target->Set(context,
              FIXED_ONE_BYTE_STRING(isolate, "nameForErrorCode"),
              name_for_error_code).Check();
}
}  // namespace http2
}  // namespace node

NODE_MODULE_CONTEXT_AWARE_INTERNAL(http2, node::http2::Initialize)