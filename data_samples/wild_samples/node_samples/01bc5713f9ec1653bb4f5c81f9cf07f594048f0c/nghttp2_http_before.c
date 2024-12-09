    if (nv->value->len != 3) {
      return NGHTTP2_ERR_HTTP_HEADER;
    }
    stream->status_code = (int16_t)parse_uint(nv->value->base, nv->value->len);
    if (stream->status_code == -1) {
      return NGHTTP2_ERR_HTTP_HEADER;
    }
    break;
  }
  case NGHTTP2_TOKEN_CONTENT_LENGTH: {
    if (stream->status_code == 204) {
      /* content-length header field in 204 response is prohibited by
         RFC 7230.  But some widely used servers send content-length:
         0.  Until they get fixed, we ignore it. */
      if (stream->content_length != -1) {
        /* Found multiple content-length field */
        return NGHTTP2_ERR_HTTP_HEADER;
      }