Http2Session::~Http2Session() {
  CHECK(!is_in_scope());
  Debug(this, "freeing nghttp2 session");
  // Ensure that all `Http2Stream` instances and the memory they hold
  // on to are destroyed before the nghttp2 session is.
  for (const auto& [id, stream] : streams_) {
    stream->Detach();
  }
  streams_.clear();
  // Explicitly reset session_ so the subsequent
  // current_nghttp2_memory_ check passes.
  session_.reset();
  CHECK_EQ(current_nghttp2_memory_, 0);