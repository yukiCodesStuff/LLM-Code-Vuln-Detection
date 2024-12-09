  }

  CHECK_LE(static_cast<size_t>(nread), buf.len);

  Local<Object> obj = Buffer::New(env, buf.base, nread).ToLocalChecked();
  stream->CallJSOnreadMethod(nread, obj);
}

