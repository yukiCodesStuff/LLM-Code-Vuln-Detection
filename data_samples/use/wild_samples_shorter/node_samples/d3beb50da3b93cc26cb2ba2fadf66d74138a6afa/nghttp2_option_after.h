  NGHTTP2_OPT_MAX_DEFLATE_DYNAMIC_TABLE_SIZE = 1 << 9,
  NGHTTP2_OPT_NO_CLOSED_STREAMS = 1 << 10,
  NGHTTP2_OPT_MAX_OUTBOUND_ACK = 1 << 11,
  NGHTTP2_OPT_MAX_SETTINGS = 1 << 12,
} nghttp2_option_flag;

/**
 * Struct to store option values for nghttp2_session.
   * NGHTTP2_OPT_MAX_OUTBOUND_ACK
   */
  size_t max_outbound_ack;
  /**
   * NGHTTP2_OPT_MAX_SETTINGS
   */
  size_t max_settings;
  /**
   * Bitwise OR of nghttp2_option_flag to determine that which fields
   * are specified.
   */