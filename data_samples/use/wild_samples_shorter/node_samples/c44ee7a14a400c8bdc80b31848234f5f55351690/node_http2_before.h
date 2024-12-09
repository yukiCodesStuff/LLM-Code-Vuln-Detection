                        void* user_data);
};


class Http2Session : public AsyncWrap, public StreamListener {
 public:
  Http2Session(Environment* env,
  // The underlying nghttp2_session handle
  nghttp2_session* session_;

  // The session type: client or server
  nghttp2_session_type session_type_;

  // The maximum number of header pairs permitted for streams on this session