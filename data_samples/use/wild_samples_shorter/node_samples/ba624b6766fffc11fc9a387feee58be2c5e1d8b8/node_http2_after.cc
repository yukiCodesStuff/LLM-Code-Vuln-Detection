void Http2Session::OnStreamAfterWrite(WriteWrap* w, int status) {
  Debug(this, "write finished with status %d", status);

  CHECK_NE(flags_ & SESSION_STATE_WRITE_IN_PROGRESS, 0);
  flags_ &= ~SESSION_STATE_WRITE_IN_PROGRESS;

  // Inform all pending writes about their completion.
  ClearOutgoing(status);

  if ((flags_ & SESSION_STATE_READING_STOPPED) &&
      nghttp2_session_want_read(session_)) {
    flags_ &= ~SESSION_STATE_READING_STOPPED;
    stream_->ReadStart();
  }

  if (!(flags_ & SESSION_STATE_WRITE_SCHEDULED)) {
    // Schedule a new write if nghttp2 wants to send data.
    MaybeScheduleWrite();
  }
}

void Http2Session::MaybeStopReading() {
  if (flags_ & SESSION_STATE_READING_STOPPED) return;
  int want_read = nghttp2_session_want_read(session_);
  Debug(this, "wants read? %d", want_read);
  if (want_read == 0 || (flags_ & SESSION_STATE_WRITE_IN_PROGRESS)) {
    flags_ |= SESSION_STATE_READING_STOPPED;
    stream_->ReadStop();
  }
}

// Unset the sending state, finish up all current writes, and reset
// storage for data and metadata that was associated with these writes.

  chunks_sent_since_last_write_++;

  CHECK_EQ(flags_ & SESSION_STATE_WRITE_IN_PROGRESS, 0);
  flags_ |= SESSION_STATE_WRITE_IN_PROGRESS;
  StreamWriteResult res = underlying_stream()->Write(*bufs, count);
  if (!res.async) {
    flags_ &= ~SESSION_STATE_WRITE_IN_PROGRESS;
    ClearOutgoing(res.err);
  }

  MaybeStopReading();