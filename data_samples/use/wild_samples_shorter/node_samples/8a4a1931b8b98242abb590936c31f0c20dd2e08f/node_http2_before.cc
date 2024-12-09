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


inline int32_t GetFrameID(const nghttp2_frame* frame) {
      nghttp2_session_consume_stream(handle, id, avail);
    else
      stream->inbound_consumed_data_while_paused_ += avail;
  } while (len != 0);

  return 0;
}

// Called by nghttp2 when it needs to determine how much padding to use in
  size_t offset = buf.base - session->stream_buf_.base;

  // Verify that the data offset is inside the current read buffer.
  CHECK_LE(offset, session->stream_buf_.len);
  CHECK_LE(offset + buf.len, session->stream_buf_.len);

  stream->CallJSOnreadMethod(nread, ab, offset);
    stream_->ReadStart();
  }

  if (!(flags_ & SESSION_STATE_WRITE_SCHEDULED)) {
    // Schedule a new write if nghttp2 wants to send data.
    MaybeScheduleWrite();
  }

  if (outgoing_buffers_.size() > 0) {
    outgoing_storage_.clear();

    std::vector<nghttp2_stream_write> current_outgoing_buffers_;
    current_outgoing_buffers_.swap(outgoing_buffers_);
    for (const nghttp2_stream_write& wr : current_outgoing_buffers_) {
  }
}

// Queue a given block of data for sending. This always creates a copy,
// so it is used for the cases in which nghttp2 requests sending of a
// small chunk of data.
void Http2Session::CopyDataIntoOutgoing(const uint8_t* src, size_t src_length) {
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

  if (frame->data.padlen > 0) {
    // Send padding if that was requested.
    session->outgoing_buffers_.emplace_back(nghttp2_stream_write {
      uv_buf_init(const_cast<char*>(zero_bytes_256), frame->data.padlen - 1)
    });
  }

  Http2Scope h2scope(this);
  CHECK_NOT_NULL(stream_);
  Debug(this, "receiving %d bytes", nread);
  CHECK_EQ(stream_buf_allocation_.size(), 0);
  CHECK(stream_buf_ab_.IsEmpty());
  AllocatedBuffer buf(env(), buf_);

  // Only pass data on if nread > 0
  if (nread <= 0) {
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
  // to copy memory.
  stream_buf_allocation_ = std::move(buf);

  statistics_.data_received += nread;
  ssize_t ret = Write(&stream_buf_, 1);

  if (UNLIKELY(ret < 0)) {
    Debug(this, "fatal error receiving data: %d", ret);
    Local<Value> arg = Integer::New(isolate, ret);