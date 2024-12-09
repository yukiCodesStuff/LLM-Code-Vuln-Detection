 */
#define NGHTTP2_CLIENT_MAGIC_LEN 24

/**
 * @macro
 *
 * The default max number of settings per SETTINGS frame
 */
#define NGHTTP2_DEFAULT_MAX_SETTINGS 32

/**
 * @enum
 *
 * Error codes used in this library.  The code range is [-999, -500],
   * receives an other type of frame.
   */
  NGHTTP2_ERR_SETTINGS_EXPECTED = -536,
  /**
   * When a local endpoint receives too many settings entries
   * in a single SETTINGS frame.
   */
  NGHTTP2_ERR_TOO_MANY_SETTINGS = -537,
  /**
   * The errors < :enum:`NGHTTP2_ERR_FATAL` mean that the library is
   * under unexpected condition and processing was terminated (e.g.,
   * out of memory).  If application receives this error code, it must
NGHTTP2_EXTERN void nghttp2_option_set_max_outbound_ack(nghttp2_option *option,
                                                        size_t val);

/**
 * @function
 *
 * This function sets the maximum number of SETTINGS entries per
 * SETTINGS frame that will be accepted. If more than those entries
 * are received, the peer is considered to be misbehaving and session
 * will be closed. The default value is 32.
 */
NGHTTP2_EXTERN void nghttp2_option_set_max_settings(nghttp2_option *option,
                                                    size_t val);

/**
 * @function
 *
 * Initializes |*session_ptr| for client use.  The all members of