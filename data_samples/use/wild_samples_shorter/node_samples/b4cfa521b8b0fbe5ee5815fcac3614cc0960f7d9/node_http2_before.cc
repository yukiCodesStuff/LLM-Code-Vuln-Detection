    return;

  std::vector<nghttp2_header> headers(stream->move_headers());

  // The headers are passed in above as a queue of nghttp2_header structs.
  // The following converts that into a JS array with the structure:
  // [name1, value1, name2, value2, name3, value3, name3, value4] and so on.
  if (session_ == nullptr)
    return;
  Debug(this, "tearing down stream");
  session_->RemoveStream(this);
  session_ = nullptr;
}

void Http2Stream::StartHeaders(nghttp2_headers_category category) {
  Debug(this, "starting headers, category: %d", id_, category);
  CHECK(!this->IsDestroyed());
  current_headers_length_ = 0;
  current_headers_.clear();
  current_headers_category_ = category;
}
  CHECK(!this->IsDestroyed());
  if (this->statistics_.first_header == 0)
    this->statistics_.first_header = uv_hrtime();
  size_t length = nghttp2_rcbuf_get_buf(name).len +
                  nghttp2_rcbuf_get_buf(value).len + 32;
  // A header can only be added if we have not exceeded the maximum number
  // of headers and the session has memory available for it.
  if (!session_->IsAvailableSessionMemory(length) ||
      current_headers_.size() == max_header_pairs_ ||
  nghttp2_rcbuf_incref(name);
  nghttp2_rcbuf_incref(value);
  current_headers_length_ += length;
  return true;
}

// A Provider is the thing that provides outbound DATA frame data.