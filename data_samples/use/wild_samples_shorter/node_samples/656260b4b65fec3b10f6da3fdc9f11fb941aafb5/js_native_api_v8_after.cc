  if (!buf) {
    CHECK_ARG(env, result);
    *result = val.As<v8::String>()->Length();
  } else if (bufsize != 0) {
    int copied =
        val.As<v8::String>()->WriteOneByte(env->isolate,
                                           reinterpret_cast<uint8_t*>(buf),
                                           0,
    if (result != nullptr) {
      *result = copied;
    }
  } else if (result != nullptr) {
    *result = 0;
  }

  return napi_clear_last_error(env);
}
  if (!buf) {
    CHECK_ARG(env, result);
    *result = val.As<v8::String>()->Utf8Length(env->isolate);
  } else if (bufsize != 0) {
    int copied = val.As<v8::String>()->WriteUtf8(
        env->isolate,
        buf,
        bufsize - 1,
    if (result != nullptr) {
      *result = copied;
    }
  } else if (result != nullptr) {
    *result = 0;
  }

  return napi_clear_last_error(env);
}
    CHECK_ARG(env, result);
    // V8 assumes UTF-16 length is the same as the number of characters.
    *result = val.As<v8::String>()->Length();
  } else if (bufsize != 0) {
    int copied = val.As<v8::String>()->Write(env->isolate,
                                             reinterpret_cast<uint16_t*>(buf),
                                             0,
                                             bufsize - 1,
    if (result != nullptr) {
      *result = copied;
    }
  } else if (result != nullptr) {
    *result = 0;
  }

  return napi_clear_last_error(env);
}