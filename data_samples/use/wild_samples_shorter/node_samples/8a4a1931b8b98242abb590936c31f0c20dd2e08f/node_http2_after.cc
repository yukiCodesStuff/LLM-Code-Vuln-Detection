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


inline int32_t GetFrameID(const nghttp2_frame* frame) {
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

  return 0;
}

// Called by nghttp2 when it needs to determine how much padding to use in
  size_t offset = buf.base - session->stream_buf_.base;

  // Verify that the data offset is inside the current read buffer.
  CHECK_GE(offset, session->stream_buf_offset_);
  CHECK_LE(offset, session->stream_buf_.len);
  CHECK_LE(offset + buf.len, session->stream_buf_.len);

  stream->CallJSOnreadMethod(nread, ab, offset);
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

  if (outgoing_buffers_.size() > 0) {
    outgoing_storage_.clear();
    outgoing_length_ = 0;

    std::vector<nghttp2_stream_write> current_outgoing_buffers_;
    current_outgoing_buffers_.swap(outgoing_buffers_);
    for (const nghttp2_stream_write& wr : current_outgoing_buffers_) {
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

  if (frame->data.padlen > 0) {
    // Send padding if that was requested.
    session->PushOutgoingBuffer(nghttp2_stream_write {
      uv_buf_init(const_cast<char*>(zero_bytes_256), frame->data.padlen - 1)
    });
  }

  Http2Scope h2scope(this);
  CHECK_NOT_NULL(stream_);
  Debug(this, "receiving %d bytes", nread);
  AllocatedBuffer buf(env(), buf_);

  // Only pass data on if nread > 0
  if (nread <= 0) {
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
  // to copy memory.
  stream_buf_allocation_ = std::move(buf);

  ssize_t ret = ConsumeHTTP2Data();

  if (UNLIKELY(ret < 0)) {
    Debug(this, "fatal error receiving data: %d", ret);
    Local<Value> arg = Integer::New(isolate, ret);