  // misbehaving peer. This counter is reset once new streams are being
  // accepted again.
  int32_t rejected_stream_count_ = 0;

  void CopyDataIntoOutgoing(const uint8_t* src, size_t src_length);
  void ClearOutgoing(int status);
