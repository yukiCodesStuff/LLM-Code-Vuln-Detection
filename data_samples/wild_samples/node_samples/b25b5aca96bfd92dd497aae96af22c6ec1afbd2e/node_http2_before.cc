Http2Session::~Http2Session() {
  CHECK(!is_in_scope());
  Debug(this, "freeing nghttp2 session");
  // Explicitly reset session_ so the subsequent
  // current_nghttp2_memory_ check passes.
  session_.reset();
  CHECK_EQ(current_nghttp2_memory_, 0);
}

void Http2Session::FetchAllowedRemoteCustomSettings() {
  AliasedUint32Array& buffer = http2_state_->settings_buffer;
  uint32_t numAddSettings = buffer[IDX_SETTINGS_COUNT + 1];
  if (numAddSettings > 0) {
    nghttp2_settings_entry* entries = remote_custom_settings_.entries;
    uint32_t offset = IDX_SETTINGS_COUNT + 1 + 1;
    size_t count = 0;
    for (uint32_t i = 0; i < numAddSettings; i++) {
      uint32_t key =
          (buffer[offset + i * 2 + 0] & 0xffff) |
          (1
           << 16);  // setting the bit 16 indicates, that no values has been set
      entries[count++] = nghttp2_settings_entry{(int32_t)key, 0};
    }
    remote_custom_settings_.number = count;
  }