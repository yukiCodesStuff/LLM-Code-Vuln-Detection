 */
#define NGHTTP2_CLIENT_MAGIC_LEN 24

/**
 * @enum
 *
 * Error codes used in this library.  The code range is [-999, -500],
   * receives an other type of frame.
   */
  NGHTTP2_ERR_SETTINGS_EXPECTED = -536,
  /**
   * The errors < :enum:`NGHTTP2_ERR_FATAL` mean that the library is
   * under unexpected condition and processing was terminated (e.g.,
   * out of memory).  If application receives this error code, it must
NGHTTP2_EXTERN void nghttp2_option_set_max_outbound_ack(nghttp2_option *option,
                                                        size_t val);

/**
 * @function
 *
 * Initializes |*session_ptr| for client use.  The all members of