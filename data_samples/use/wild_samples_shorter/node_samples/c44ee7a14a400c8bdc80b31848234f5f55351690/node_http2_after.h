                        void* user_data);
};

// Indices for js_fields_, which serves as a way to communicate data with JS
// land fast. In particular, we store information about the number/presence
// of certain event listeners in JS, and skip calls from C++ into JS if they
// are missing.
enum SessionUint8Fields {
  kBitfield,  // See below
  kSessionPriorityListenerCount,
  kSessionFrameErrorListenerCount,
  kSessionUint8FieldCount
};

enum SessionBitfieldFlags {
  kSessionHasRemoteSettingsListeners,
  kSessionRemoteSettingsIsUpToDate,
  kSessionHasPingListeners,
  kSessionHasAltsvcListeners
};

class Http2Session : public AsyncWrap, public StreamListener {
 public:
  Http2Session(Environment* env,
  // The underlying nghttp2_session handle
  nghttp2_session* session_;

  // JS-accessible numeric fields, as indexed by SessionUint8Fields.
  uint8_t js_fields_[kSessionUint8FieldCount] = {};

  // The session type: client or server
  nghttp2_session_type session_type_;

  // The maximum number of header pairs permitted for streams on this session