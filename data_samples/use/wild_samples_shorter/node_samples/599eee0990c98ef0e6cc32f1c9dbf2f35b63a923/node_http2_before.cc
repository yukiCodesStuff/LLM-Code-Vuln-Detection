    return;
  }

  CHECK(!session->stream_buf_ab_.IsEmpty());

  // There is a single large array buffer for the entire data read from the
  // network; create a slice of that array buffer and emit it as the
  // received data buffer.
  CHECK_LE(offset, session->stream_buf_.len);
  CHECK_LE(offset + buf.len, session->stream_buf_.len);

  stream->CallJSOnreadMethod(nread, session->stream_buf_ab_, offset);
}


// Called by OnFrameReceived to notify JavaScript land that a complete
  Http2Scope h2scope(this);
  CHECK_NOT_NULL(stream_);
  Debug(this, "receiving %d bytes", nread);
  CHECK(stream_buf_ab_.IsEmpty());
  AllocatedBuffer buf(env(), buf_);

  // Only pass data on if nread > 0
    // We use `nread` instead of `buf.size()` here, because the buffer is
    // cleared as part of the `.ToArrayBuffer()` call below.
    DecrementCurrentSessionMemory(nread);
    stream_buf_ab_ = Local<ArrayBuffer>();
    stream_buf_ = uv_buf_init(nullptr, 0);
  });

  // Make sure that there was no read previously active.

  Isolate* isolate = env()->isolate();

  // Create an array buffer for the read data. DATA frames will be emitted
  // as slices of this array buffer to avoid having to copy memory.
  stream_buf_ab_ = buf.ToArrayBuffer();

  statistics_.data_received += nread;
  ssize_t ret = Write(&stream_buf_, 1);
