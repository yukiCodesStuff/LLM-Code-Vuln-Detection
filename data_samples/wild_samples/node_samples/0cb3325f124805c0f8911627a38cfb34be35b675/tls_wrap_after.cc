  if (i != count) {
    int err;
    Local<Value> arg = GetSSLError(written, &err, &error_);
    if (!arg.IsEmpty()) {
      current_write_ = nullptr;
      return UV_EPROTO;
    }