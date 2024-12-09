void nghttp2_frame_altsvc_free(nghttp2_extension *frame, nghttp2_mem *mem) {
  nghttp2_ext_altsvc *altsvc;

  altsvc = frame->payload;
  if (altsvc == NULL) {
    return;
  }