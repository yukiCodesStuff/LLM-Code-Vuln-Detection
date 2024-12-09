  if (try_write) {
    data_size = StringBytes::Write(env->isolate(),
                                   stack_storage,
                                   storage_size,
                                   string,
                                   enc);
    buf = uv_buf_init(stack_storage, data_size);

    uv_buf_t* bufs = &buf;
    size_t count = 1;
    const int err = DoTryWrite(&bufs, &count);
    // Keep track of the bytes written here, because we're taking a shortcut
    // by using `DoTryWrite()` directly instead of using the utilities
    // provided by `Write()`.
    synchronously_written = count == 0 ? data_size : data_size - buf.len;
    bytes_written_ += synchronously_written;

    // Immediate failure or success
    if (err != 0 || count == 0) {
      SetWriteResult(StreamWriteResult { false, err, nullptr, data_size });
      return err;
    }