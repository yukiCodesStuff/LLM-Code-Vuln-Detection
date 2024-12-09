nghttp2_session_set_stream_user_data(nghttp2_session *session,
                                     int32_t stream_id, void *stream_user_data);

/**
 * @function
 *
 * Sets |user_data| to |session|, overwriting the existing user data
 * specified in `nghttp2_session_client_new()`, or
 * `nghttp2_session_server_new()`.
 */
NGHTTP2_EXTERN void nghttp2_session_set_user_data(nghttp2_session *session,
                                                  void *user_data);

/**
 * @function
 *
 * Returns the number of frames in the outbound queue.  This does not