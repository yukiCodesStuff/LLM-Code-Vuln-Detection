  /* The maximum length of header block to send.  Calculated by the
     same way as nghttp2_hd_deflate_bound() does. */
  size_t max_send_header_block_length;
  /* Next Stream ID. Made unsigned int to detect >= (1 << 31). */
  uint32_t next_stream_id;
  /* The last stream ID this session initiated.  For client session,
     this is the last stream ID it has sent.  For server session, it
                                                  uint32_t error_code,
                                                  const char *reason);

#endif /* NGHTTP2_SESSION_H */