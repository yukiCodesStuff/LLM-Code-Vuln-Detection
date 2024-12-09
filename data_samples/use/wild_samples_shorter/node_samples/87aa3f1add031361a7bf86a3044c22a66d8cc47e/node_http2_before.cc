      frame->hd.flags & NGHTTP2_FLAG_END_STREAM) {
    stream->EmitRead(UV_EOF);
  } else if (frame->hd.length == 0) {
    return 1;  // Consider 0-length frame without END_STREAM an error.
  }
  return 0;
}
