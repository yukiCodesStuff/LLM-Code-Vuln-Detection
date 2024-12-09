void nghttp2_frame_altsvc_free(nghttp2_extension *frame, nghttp2_mem *mem) {
  nghttp2_ext_altsvc *altsvc;

  altsvc = frame->payload;
  /* We use the same buffer for altsvc->origin and
     altsvc->field_value. */
  nghttp2_mem_free(mem, altsvc->origin);
}

size_t nghttp2_frame_priority_len(uint8_t flags) {
  if (flags & NGHTTP2_FLAG_PRIORITY) {
    return NGHTTP2_PRIORITY_SPECLEN;
  }