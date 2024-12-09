  SESSION_STATE_CLOSED = 0x4,
  SESSION_STATE_CLOSING = 0x8,
  SESSION_STATE_SENDING = 0x10,
};

typedef uint32_t(*get_setting)(nghttp2_session* session,
                               nghttp2_settings_id id);