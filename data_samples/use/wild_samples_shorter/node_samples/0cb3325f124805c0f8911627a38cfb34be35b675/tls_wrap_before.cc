  if (i != count) {
    int err;
    Local<Value> arg = GetSSLError(written, &err, &error_);
    if (!arg.IsEmpty())
      return UV_EPROTO;

    pending_cleartext_input_.insert(pending_cleartext_input_.end(),
                                    &bufs[i],
                                    &bufs[count]);