                                        size_t maxPayloadLen) {
  if (frameLen == 0) return 0;
  Debug(this, "using callback to determine padding");
  Isolate* isolate = env()->isolate();
  HandleScope handle_scope(isolate);
  Local<Context> context = env()->context();
  Context::Scope context_scope(context);

  AliasedUint32Array& buffer = env()->http2_state()->padding_buffer;
  buffer[PADDING_BUF_FRAME_LENGTH] = frameLen;
  buffer[PADDING_BUF_MAX_PAYLOAD_LENGTH] = maxPayloadLen;
  buffer[PADDING_BUF_RETURN_VALUE] = frameLen;
  MakeCallback(env()->http2session_on_select_padding_function(), 0, nullptr);
  uint32_t retval = buffer[PADDING_BUF_RETURN_VALUE];
  retval = std::min(retval, static_cast<uint32_t>(maxPayloadLen));
  retval = std::max(retval, static_cast<uint32_t>(frameLen));
  Debug(this, "using padding size %d", retval);
  return retval;
}


// Write data received from the i/o stream to the underlying nghttp2_session.
// On each call to nghttp2_session_mem_recv, nghttp2 will begin calling the
// various callback functions. Each of these will typically result in a call
// out to JavaScript so this particular function is rather hot and can be
// quite expensive. This is a potential performance optimization target later.
ssize_t Http2Session::ConsumeHTTP2Data() {
  CHECK_NOT_NULL(stream_buf_.base);
  CHECK_LT(stream_buf_offset_, stream_buf_.len);
  size_t read_len = stream_buf_.len - stream_buf_offset_;

  // multiple side effects.
  Debug(this, "receiving %d bytes [wants data? %d]",
        read_len,
        nghttp2_session_want_read(session_));
  flags_ &= ~SESSION_STATE_NGHTTP2_RECV_PAUSED;
  ssize_t ret =
    nghttp2_session_mem_recv(session_,
                             reinterpret_cast<uint8_t*>(stream_buf_.base) +
                                 stream_buf_offset_,
                             read_len);
  CHECK_NE(ret, NGHTTP2_ERR_NOMEM);

  if (flags_ & SESSION_STATE_NGHTTP2_RECV_PAUSED) {
    CHECK_NE(flags_ & SESSION_STATE_READING_STOPPED, 0);

    CHECK_GT(ret, 0);
    CHECK_LE(static_cast<size_t>(ret), read_len);

    if (static_cast<size_t>(ret) < read_len) {
      // Mark the remainder of the data as available for later consumption.
      stream_buf_offset_ += ret;
      return ret;
    }
  }

  // We are done processing the current input chunk.
  DecrementCurrentSessionMemory(stream_buf_.len);
  stream_buf_offset_ = 0;
  stream_buf_ab_.Reset();
  stream_buf_allocation_.clear();
  stream_buf_ = uv_buf_init(nullptr, 0);

  if (ret < 0)
    return ret;

  // Send any data that was queued up while processing the received data.
  if (!IsDestroyed()) {
    SendPendingData();
  }
  return ret;
}
  do {
    uv_buf_t buf = stream->EmitAlloc(len);
    ssize_t avail = len;
    if (static_cast<ssize_t>(buf.len) < avail)
      avail = buf.len;

    // `buf.base == nullptr` is the default Http2StreamListener's way
    // of saying that it wants a pointer to the raw original.
    // Since it has access to the original socket buffer from which the data
    // was read in the first place, it can use that to minimize ArrayBuffer
    // allocations.
    if (LIKELY(buf.base == nullptr))
      buf.base = reinterpret_cast<char*>(const_cast<uint8_t*>(data));
    else
      memcpy(buf.base, data, avail);
    data += avail;
    len -= avail;
    stream->EmitRead(avail, buf);

    // If the stream owner (e.g. the JS Http2Stream) wants more data, just
    // tell nghttp2 that all data has been consumed. Otherwise, defer until
    // more data is being requested.
    if (stream->IsReading())
      nghttp2_session_consume_stream(handle, id, avail);
    else
      stream->inbound_consumed_data_while_paused_ += avail;

    // If we have a gathered a lot of data for output, try sending it now.
    if (session->outgoing_length_ > 4096) session->SendPendingData();
  } while (len != 0);

  // If we are currently waiting for a write operation to finish, we should
  // tell nghttp2 that we want to wait before we process more input data.
  if (session->flags_ & SESSION_STATE_WRITE_IN_PROGRESS) {
    session->flags_ |= SESSION_STATE_NGHTTP2_RECV_PAUSED;
    return NGHTTP2_ERR_PAUSE;
  }
  } else {
    ab = PersistentToLocal::Strong(session->stream_buf_ab_);
  }

  // There is a single large array buffer for the entire data read from the
  // network; create a slice of that array buffer and emit it as the
  // received data buffer.
  size_t offset = buf.base - session->stream_buf_.base;

  // Verify that the data offset is inside the current read buffer.
  CHECK_GE(offset, session->stream_buf_offset_);
  CHECK_LE(offset, session->stream_buf_.len);
  CHECK_LE(offset + buf.len, session->stream_buf_.len);

  stream->CallJSOnreadMethod(nread, ab, offset);
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
  DecrementCurrentSessionMemory(stream->current_headers_length_);
  stream->current_headers_length_ = 0;

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
      nghttp2_session_want_read(session_)) {
    flags_ &= ~SESSION_STATE_READING_STOPPED;
    stream_->ReadStart();
  }

  // If there is more incoming data queued up, consume it.
  if (stream_buf_offset_ > 0) {
    ConsumeHTTP2Data();
  }

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
  if (outgoing_buffers_.size() > 0) {
    outgoing_storage_.clear();
    outgoing_length_ = 0;

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
    for (int32_t stream_id : current_pending_rst_streams) {
      Http2Stream* stream = FindStream(stream_id);
      if (LIKELY(stream != nullptr))
        stream->FlushRstStream();
    }
  }
}

void Http2Session::PushOutgoingBuffer(nghttp2_stream_write&& write) {
  outgoing_length_ += write.buf.len;
  outgoing_buffers_.emplace_back(std::move(write));
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
  PushOutgoingBuffer(nghttp2_stream_write {
    uv_buf_init(nullptr, src_length)
  });
}
void Http2Session::CopyDataIntoOutgoing(const uint8_t* src, size_t src_length) {
  size_t offset = outgoing_storage_.size();
  outgoing_storage_.resize(offset + src_length);
  memcpy(&outgoing_storage_[offset], src, src_length);

  // Store with a base of `nullptr` initially, since future resizes
  // of the outgoing_buffers_ vector may invalidate the pointer.
  // The correct base pointers will be set later, before writing to the
  // underlying socket.
  PushOutgoingBuffer(nghttp2_stream_write {
    uv_buf_init(nullptr, src_length)
  });
}
    if (write.buf.len <= length) {
      // This write does not suffice by itself, so we can consume it completely.
      length -= write.buf.len;
      session->PushOutgoingBuffer(std::move(write));
      stream->queue_.pop();
      continue;
    }

    // Slice off `length` bytes of the first write in the queue.
    session->PushOutgoingBuffer(nghttp2_stream_write {
      uv_buf_init(write.buf.base, length)
    });
    write.buf.base += length;
    write.buf.len -= length;
    break;
  }

  if (frame->data.padlen > 0) {
    // Send padding if that was requested.
    session->PushOutgoingBuffer(nghttp2_stream_write {
      uv_buf_init(const_cast<char*>(zero_bytes_256), frame->data.padlen - 1)
    });
  }
  if (frame->data.padlen > 0) {
    // Send padding if that was requested.
    session->PushOutgoingBuffer(nghttp2_stream_write {
      uv_buf_init(const_cast<char*>(zero_bytes_256), frame->data.padlen - 1)
    });
  }

  return 0;
}
void Http2Session::OnStreamRead(ssize_t nread, const uv_buf_t& buf_) {
  HandleScope handle_scope(env()->isolate());
  Context::Scope context_scope(env()->context());
  Http2Scope h2scope(this);
  CHECK_NOT_NULL(stream_);
  Debug(this, "receiving %d bytes", nread);
  AllocatedBuffer buf(env(), buf_);

  // Only pass data on if nread > 0
  if (nread <= 0) {
    if (nread < 0) {
      PassReadErrorToPreviousListener(nread);
    }
    return;
  }
    if (nread < 0) {
      PassReadErrorToPreviousListener(nread);
    }
    return;
  }

  statistics_.data_received += nread;

  if (UNLIKELY(stream_buf_offset_ > 0)) {
    // This is a very unlikely case, and should only happen if the ReadStart()
    // call in OnStreamAfterWrite() immediately provides data. If that does
    // happen, we concatenate the data we received with the already-stored
    // pending input data, slicing off the already processed part.
    AllocatedBuffer new_buf = env()->AllocateManaged(
        stream_buf_.len - stream_buf_offset_ + nread);
    memcpy(new_buf.data(),
           stream_buf_.base + stream_buf_offset_,
           stream_buf_.len - stream_buf_offset_);
    memcpy(new_buf.data() + stream_buf_.len - stream_buf_offset_,
           buf.data(),
           nread);
    buf = std::move(new_buf);
    nread = buf.size();
    stream_buf_offset_ = 0;
    stream_buf_ab_.Reset();
    DecrementCurrentSessionMemory(stream_buf_offset_);
  }

  // Shrink to the actual amount of used data.
  buf.Resize(nread);
  IncrementCurrentSessionMemory(nread);

  // Remember the current buffer, so that OnDataChunkReceived knows the
  // offset of a DATA frame's data into the socket read buffer.
  stream_buf_ = uv_buf_init(buf.data(), nread);

  Isolate* isolate = env()->isolate();

  // Store this so we can create an ArrayBuffer for read data from it.
  // DATA frames will be emitted as slices of that ArrayBuffer to avoid having
  // to copy memory.
  stream_buf_allocation_ = std::move(buf);

  ssize_t ret = ConsumeHTTP2Data();

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
  if (UNLIKELY(stream_buf_offset_ > 0)) {
    // This is a very unlikely case, and should only happen if the ReadStart()
    // call in OnStreamAfterWrite() immediately provides data. If that does
    // happen, we concatenate the data we received with the already-stored
    // pending input data, slicing off the already processed part.
    AllocatedBuffer new_buf = env()->AllocateManaged(
        stream_buf_.len - stream_buf_offset_ + nread);
    memcpy(new_buf.data(),
           stream_buf_.base + stream_buf_offset_,
           stream_buf_.len - stream_buf_offset_);
    memcpy(new_buf.data() + stream_buf_.len - stream_buf_offset_,
           buf.data(),
           nread);
    buf = std::move(new_buf);
    nread = buf.size();
    stream_buf_offset_ = 0;
    stream_buf_ab_.Reset();
    DecrementCurrentSessionMemory(stream_buf_offset_);
  }

  // Shrink to the actual amount of used data.
  buf.Resize(nread);
  IncrementCurrentSessionMemory(nread);

  // Remember the current buffer, so that OnDataChunkReceived knows the
  // offset of a DATA frame's data into the socket read buffer.
  stream_buf_ = uv_buf_init(buf.data(), nread);

  Isolate* isolate = env()->isolate();

  // Store this so we can create an ArrayBuffer for read data from it.
  // DATA frames will be emitted as slices of that ArrayBuffer to avoid having
  // to copy memory.
  stream_buf_allocation_ = std::move(buf);

  ssize_t ret = ConsumeHTTP2Data();

  if (UNLIKELY(ret < 0)) {
    Debug(this, "fatal error receiving data: %d", ret);
    Local<Value> arg = Integer::New(isolate, ret);
    MakeCallback(env()->http2session_on_error_function(), 1, &arg);
    return;
  }