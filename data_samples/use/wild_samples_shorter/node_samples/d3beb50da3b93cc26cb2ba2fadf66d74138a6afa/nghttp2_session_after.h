  /* The maximum length of header block to send.  Calculated by the
     same way as nghttp2_hd_deflate_bound() does. */
  size_t max_send_header_block_length;
  /* The maximum number of settings accepted per SETTINGS frame. */
  size_t max_settings;
  /* Next Stream ID. Made unsigned int to detect >= (1 << 31). */
  uint32_t next_stream_id;
  /* The last stream ID this session initiated.  For client session,
     this is the last stream ID it has sent.  For server session, it
                                                  uint32_t error_code,
                                                  const char *reason);

/*
 * Accumulates received bytes |delta_size| for connection-level flow
 * control and decides whether to send WINDOW_UPDATE to the
 * connection.  If NGHTTP2_OPT_NO_AUTO_WINDOW_UPDATE is set,
 * WINDOW_UPDATE will not be sent.
 *
 * This function returns 0 if it succeeds, or one of the following
 * negative error codes:
 *
 * NGHTTP2_ERR_NOMEM
 *     Out of memory.
 */
int nghttp2_session_update_recv_connection_window_size(nghttp2_session *session,
                                                       size_t delta_size);

/*
 * Accumulates received bytes |delta_size| for stream-level flow
 * control and decides whether to send WINDOW_UPDATE to that stream.
 * If NGHTTP2_OPT_NO_AUTO_WINDOW_UPDATE is set, WINDOW_UPDATE will not
 * be sent.
 *
 * This function returns 0 if it succeeds, or one of the following
 * negative error codes:
 *
 * NGHTTP2_ERR_NOMEM
 *     Out of memory.
 */
int nghttp2_session_update_recv_stream_window_size(nghttp2_session *session,
                                                   nghttp2_stream *stream,
                                                   size_t delta_size,
                                                   int send_window_update);

#endif /* NGHTTP2_SESSION_H */