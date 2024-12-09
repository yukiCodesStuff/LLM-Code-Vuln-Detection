
    // Immediate failure or success
    if (err != 0 || count == 0) {
      SetWriteResult(StreamWriteResult { false, err, nullptr, data_size });
      return err;
    }

    // Partial write