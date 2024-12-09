      return NGHTTP2_ERR_HTTP_HEADER;
    }
    stream->status_code = (int16_t)parse_uint(nv->value->base, nv->value->len);
    if (stream->status_code == -1) {
      return NGHTTP2_ERR_HTTP_HEADER;
    }
    break;
  }