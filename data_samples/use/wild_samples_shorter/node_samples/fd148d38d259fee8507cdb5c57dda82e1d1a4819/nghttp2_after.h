NGHTTP2_EXTERN void nghttp2_option_set_no_closed_streams(nghttp2_option *option,
                                                         int val);

/**
 * @function
 *
 * This function sets the maximum number of outgoing SETTINGS ACK and
 * PING ACK frames retained in :type:`nghttp2_session` object.  If
 * more than those frames are retained, the peer is considered to be
 * misbehaving and session will be closed.  The default value is 1000.
 */
NGHTTP2_EXTERN void nghttp2_option_set_max_outbound_ack(nghttp2_option *option,
                                                        size_t val);

/**
 * @function
 *
 * Initializes |*session_ptr| for client use.  The all members of