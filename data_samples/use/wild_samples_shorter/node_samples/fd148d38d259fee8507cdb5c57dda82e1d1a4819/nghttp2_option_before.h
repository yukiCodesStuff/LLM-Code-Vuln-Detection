  NGHTTP2_OPT_MAX_SEND_HEADER_BLOCK_LENGTH = 1 << 8,
  NGHTTP2_OPT_MAX_DEFLATE_DYNAMIC_TABLE_SIZE = 1 << 9,
  NGHTTP2_OPT_NO_CLOSED_STREAMS = 1 << 10,
} nghttp2_option_flag;

/**
 * Struct to store option values for nghttp2_session.
   * NGHTTP2_OPT_MAX_DEFLATE_DYNAMIC_TABLE_SIZE
   */
  size_t max_deflate_dynamic_table_size;
  /**
   * Bitwise OR of nghttp2_option_flag to determine that which fields
   * are specified.
   */