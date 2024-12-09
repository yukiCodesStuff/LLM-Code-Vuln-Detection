      frame->hd.flags & NGHTTP2_FLAG_END_STREAM) {
    stream->EmitRead(UV_EOF);
  } else if (frame->hd.length == 0) {
    if (invalid_frame_count_++ > js_fields_->max_invalid_frames) {
      Debug(this, "rejecting empty-frame-without-END_STREAM flood\n");
      // Consider a flood of 0-length frames without END_STREAM an error.
      return 1;
    }
  }