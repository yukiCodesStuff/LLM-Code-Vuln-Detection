    if (Buffer::HasInstance(chunk)) {
      bufs[i].base = Buffer::Data(chunk);
      bufs[i].len = Buffer::Length(chunk);
      bytes += bufs[i].len;
      continue;
    }

    // Write string
    offset = ROUND_UP(offset, WriteWrap::kAlignSize);
    CHECK_LT(offset, storage_size);
    char* str_storage = req_wrap->Extra(offset);
    size_t str_size = storage_size - offset;

    Local<String> string = chunk->ToString(env->isolate());
    enum encoding encoding = ParseEncoding(env->isolate(),
                                           chunks->Get(i * 2 + 1));
    str_size = StringBytes::Write(env->isolate(),
                                  str_storage,
                                  str_size,
                                  string,
                                  encoding);
    bufs[i].base = str_storage;
    bufs[i].len = str_size;
    offset += str_size;
    bytes += str_size;
  }

  int err = DoWrite(req_wrap, bufs, count, nullptr);

  // Deallocate space
  if (bufs != bufs_)
    delete[] bufs;

  req_wrap->object()->Set(env->async(), True(env->isolate()));
  req_wrap->object()->Set(env->bytes_string(),
                          Number::New(env->isolate(), bytes));
  const char* msg = Error();
  if (msg != nullptr) {
    req_wrap_obj->Set(env->error_string(), OneByteString(env->isolate(), msg));
    ClearError();
  }

  if (err)
    req_wrap->Dispose();

  return err;
}




int StreamBase::WriteBuffer(const FunctionCallbackInfo<Value>& args) {
  CHECK(args[0]->IsObject());
  CHECK(Buffer::HasInstance(args[1]));
  Environment* env = Environment::GetCurrent(args);

  Local<Object> req_wrap_obj = args[0].As<Object>();
  const char* data = Buffer::Data(args[1]);
  size_t length = Buffer::Length(args[1]);

  WriteWrap* req_wrap;
  uv_buf_t buf;
  buf.base = const_cast<char*>(data);
  buf.len = length;

  // Try writing immediately without allocation
  uv_buf_t* bufs = &buf;
  size_t count = 1;
  int err = DoTryWrite(&bufs, &count);
  if (err != 0)
    goto done;
  if (count == 0)
    goto done;
  CHECK_EQ(count, 1);

  // Allocate, or write rest
  req_wrap = WriteWrap::New(env, req_wrap_obj, this, AfterWrite);

  err = DoWrite(req_wrap, bufs, count, nullptr);
  req_wrap_obj->Set(env->async(), True(env->isolate()));

  if (err)
    req_wrap->Dispose();

 done:
  const char* msg = Error();
  if (msg != nullptr) {
    req_wrap_obj->Set(env->error_string(), OneByteString(env->isolate(), msg));
    ClearError();
  }
  req_wrap_obj->Set(env->bytes_string(),
                    Integer::NewFromUnsigned(env->isolate(), length));
  return err;
}


template <enum encoding enc>
int StreamBase::WriteString(const FunctionCallbackInfo<Value>& args) {
  Environment* env = Environment::GetCurrent(args);
  CHECK(args[0]->IsObject());
  CHECK(args[1]->IsString());

  Local<Object> req_wrap_obj = args[0].As<Object>();
  Local<String> string = args[1].As<String>();
  Local<Object> send_handle_obj;
  if (args[2]->IsObject())
    send_handle_obj = args[2].As<Object>();

  int err;

  // Compute the size of the storage that the string will be flattened into.
  // For UTF8 strings that are very long, go ahead and take the hit for
  // computing their actual size, rather than tripling the storage.
  size_t storage_size;
  if (enc == UTF8 && string->Length() > 65535)
    storage_size = StringBytes::Size(env->isolate(), string, enc);
  else
    storage_size = StringBytes::StorageSize(env->isolate(), string, enc);

  if (storage_size > INT_MAX)
    return UV_ENOBUFS;

  // Try writing immediately if write size isn't too big
  WriteWrap* req_wrap;
  char* data;
  char stack_storage[16384];  // 16kb
  size_t data_size;
  uv_buf_t buf;

  bool try_write = storage_size <= sizeof(stack_storage) &&
                   (!IsIPCPipe() || send_handle_obj.IsEmpty());
  if (try_write) {
    data_size = StringBytes::Write(env->isolate(),
                                   stack_storage,
                                   storage_size,
                                   string,
                                   enc);
    buf = uv_buf_init(stack_storage, data_size);

    uv_buf_t* bufs = &buf;
    size_t count = 1;
    err = DoTryWrite(&bufs, &count);

    // Failure
    if (err != 0)
      goto done;

    // Success
    if (count == 0)
      goto done;

    // Partial write
    CHECK_EQ(count, 1);
  }

  req_wrap = WriteWrap::New(env, req_wrap_obj, this, AfterWrite, storage_size);

  data = req_wrap->Extra();

  if (try_write) {
    // Copy partial data
    memcpy(data, buf.base, buf.len);
    data_size = buf.len;
  } else {
    // Write it
    data_size = StringBytes::Write(env->isolate(),
                                   data,
                                   storage_size,
                                   string,
                                   enc);
  }