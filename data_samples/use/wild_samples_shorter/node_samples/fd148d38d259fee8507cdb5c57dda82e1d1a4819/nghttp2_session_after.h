   response frames are stacked up, which leads to memory exhaustion.
   The value selected here is arbitrary, but safe value and if we have
   these frames in this number, it is considered suspicious. */
#define NGHTTP2_DEFAULT_MAX_OBQ_FLOOD_ITEM 1000

/* The default value of maximum number of concurrent streams. */
#define NGHTTP2_DEFAULT_MAX_CONCURRENT_STREAMS 0xffffffffu

  size_t num_idle_streams;
  /* The number of bytes allocated for nvbuf */
  size_t nvbuflen;
  /* Counter for detecting flooding in outbound queue.  If it exceeds
     max_outbound_ack, session will be closed. */
  size_t obq_flood_counter_;
  /* The maximum number of outgoing SETTINGS ACK and PING ACK in
     outbound queue. */
  size_t max_outbound_ack;
  /* The maximum length of header block to send.  Calculated by the
     same way as nghttp2_hd_deflate_bound() does. */
  size_t max_send_header_block_length;
  /* Next Stream ID. Made unsigned int to detect >= (1 << 31). */