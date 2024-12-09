  nghttp2_ext_altsvc *altsvc;

  altsvc = frame->payload;
  /* We use the same buffer for altsvc->origin and
     altsvc->field_value. */
  nghttp2_mem_free(mem, altsvc->origin);
}