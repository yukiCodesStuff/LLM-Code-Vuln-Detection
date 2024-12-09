  SESSION_STATE_CLOSED = 0x4,
  SESSION_STATE_CLOSING = 0x8,
  SESSION_STATE_SENDING = 0x10,
  SESSION_STATE_WRITE_IN_PROGRESS = 0x20,
  SESSION_STATE_READING_STOPPED = 0x40,
};

typedef uint32_t(*get_setting)(nghttp2_session* session,
                               nghttp2_settings_id id);