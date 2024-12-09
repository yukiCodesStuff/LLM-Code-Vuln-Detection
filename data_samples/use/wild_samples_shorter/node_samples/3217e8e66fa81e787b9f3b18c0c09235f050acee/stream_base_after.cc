  }

  CHECK_LE(static_cast<size_t>(nread), buf.len);
  char* base = Realloc(buf.base, nread);

  Local<Object> obj = Buffer::New(env, base, nread).ToLocalChecked();
  stream->CallJSOnreadMethod(nread, obj);
}

