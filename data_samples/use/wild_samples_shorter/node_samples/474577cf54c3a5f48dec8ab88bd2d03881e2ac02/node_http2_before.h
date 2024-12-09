  std::vector<nghttp2_stream_write> outgoing_buffers_;
  std::vector<uint8_t> outgoing_storage_;
  std::vector<int32_t> pending_rst_streams_;

  void CopyDataIntoOutgoing(const uint8_t* src, size_t src_length);
  void ClearOutgoing(int status);
