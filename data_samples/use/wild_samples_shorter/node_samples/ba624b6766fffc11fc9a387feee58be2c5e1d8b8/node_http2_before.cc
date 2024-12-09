void Http2Session::OnStreamAfterWrite(WriteWrap* w, int status) {
  Debug(this, "write finished with status %d", status);

  // Inform all pending writes about their completion.
  ClearOutgoing(status);

  if (!(flags_ & SESSION_STATE_WRITE_SCHEDULED)) {
    // Schedule a new write if nghttp2 wants to send data.
    MaybeScheduleWrite();
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

  chunks_sent_since_last_write_++;

  StreamWriteResult res = underlying_stream()->Write(*bufs, count);
  if (!res.async) {
    ClearOutgoing(res.err);
  }

  MaybeStopReading();