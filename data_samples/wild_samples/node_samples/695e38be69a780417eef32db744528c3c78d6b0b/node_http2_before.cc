  switch (frame->hd.type) {
    case NGHTTP2_DATA:
      session->HandleDataFrame(frame);
      break;
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
void Http2Session::HandleDataFrame(const nghttp2_frame* frame) {
  int32_t id = GetFrameID(frame);
  Debug(this, "handling data frame for stream %d", id);
  Http2Stream* stream = FindStream(id);

  if (!stream->IsDestroyed() && frame->hd.flags & NGHTTP2_FLAG_END_STREAM)
    stream->EmitRead(UV_EOF);
}