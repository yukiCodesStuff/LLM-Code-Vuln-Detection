NGHTTP2_EXTERN void nghttp2_option_set_no_closed_streams(nghttp2_option *option,
                                                         int val);

/**
 * @function
 *
 * Initializes |*session_ptr| for client use.  The all members of