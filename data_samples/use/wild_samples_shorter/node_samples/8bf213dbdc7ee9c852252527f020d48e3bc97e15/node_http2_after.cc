Http2Session::~Http2Session() {
  CHECK_EQ(flags_ & SESSION_STATE_HAS_SCOPE, 0);
  Debug(this, "freeing nghttp2 session");
  for (const auto& iter : streams_)
    iter.second->session_ = nullptr;
  nghttp2_session_del(session_);
}

std::string Http2Session::diagnostic_name() const {


inline void Http2Session::RemoveStream(Http2Stream* stream) {
  if (streams_.empty() || stream == nullptr)
    return;  // Nothing to remove, item was never added?
  streams_.erase(stream->id());
  DecrementCurrentSessionMemory(stream->self_size());
}
