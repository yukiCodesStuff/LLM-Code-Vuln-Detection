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
ssize_t Http2Session::Write(const uv_buf_t* bufs, size_t nbufs) {
  size_t total = 0;
  // Note that nghttp2_session_mem_recv is a synchronous operation that
  // will trigger a number of other callbacks. Those will, in turn have
  // multiple side effects.
  for (size_t n = 0; n < nbufs; n++) {
    Debug(this, "receiving %d bytes [wants data? %d]",
          bufs[n].len,
          nghttp2_session_want_read(session_));
    ssize_t ret =
      nghttp2_session_mem_recv(session_,
                               reinterpret_cast<uint8_t*>(bufs[n].base),
                               bufs[n].len);
    CHECK_NE(ret, NGHTTP2_ERR_NOMEM);

    if (ret < 0)
      return ret;

    total += ret;
  }
  // Send any data that was queued up while processing the received data.
  if (!IsDestroyed()) {
    SendPendingData();
  }
  return total;
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
  } while (len != 0);

  return 0;
}

// Called by nghttp2 when it needs to determine how much padding to use in
// a DATA or HEADERS frame.
ssize_t Http2Session::OnSelectPadding(nghttp2_session* handle,
                                      const nghttp2_frame* frame,
                                      size_t maxPayloadLen,
                                      void* user_data) {
  Http2Session* session = static_cast<Http2Session*>(user_data);
  ssize_t padding = frame->hd.length;

  switch (session->padding_strategy_) {
    case PADDING_STRATEGY_NONE:
      // Fall-through
      break;
    case PADDING_STRATEGY_MAX:
      padding = session->OnMaxFrameSizePadding(padding, maxPayloadLen);
      break;
    case PADDING_STRATEGY_ALIGNED:
      padding = session->OnDWordAlignedPadding(padding, maxPayloadLen);
      break;
    case PADDING_STRATEGY_CALLBACK:
      padding = session->OnCallbackPadding(padding, maxPayloadLen);
      break;
  }
  } else {
    ab = PersistentToLocal::Strong(session->stream_buf_ab_);
  }

  // There is a single large array buffer for the entire data read from the
  // network; create a slice of that array buffer and emit it as the
  // received data buffer.
  size_t offset = buf.base - session->stream_buf_.base;

  // Verify that the data offset is inside the current read buffer.
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
  if (frame->data.padlen > 0) {
    // Send padding if that was requested.
    session->outgoing_buffers_.emplace_back(nghttp2_stream_write {
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