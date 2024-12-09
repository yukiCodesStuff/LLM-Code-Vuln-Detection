        frame->hd.type);
  switch (frame->hd.type) {
    case NGHTTP2_DATA:
      return session->HandleDataFrame(frame);
    case NGHTTP2_PUSH_PROMISE:
      // Intentional fall-through, handled just like headers frames
    case NGHTTP2_HEADERS:
      session->HandleHeadersFrame(frame);
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