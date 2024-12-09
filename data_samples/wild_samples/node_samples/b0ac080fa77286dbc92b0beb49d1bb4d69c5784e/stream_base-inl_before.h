             .ToLocal(&req_wrap_obj)) {
      return UV_EBUSY;
    }
    StreamReq::ResetObject(req_wrap_obj);
  }

  AsyncHooks::DefaultTriggerAsyncIdScope trigger_scope(GetAsyncWrap());
  ShutdownWrap* req_wrap = CreateShutdownWrap(req_wrap_obj);
  int err = DoShutdown(req_wrap);

  if (err != 0 && req_wrap != nullptr) {
    req_wrap->Dispose();
  }

  const char* msg = Error();
  if (msg != nullptr) {
    req_wrap_obj->Set(
        env->context(),
        env->error_string(), OneByteString(env->isolate(), msg)).Check();
    ClearError();
  }

  return err;
}

StreamWriteResult StreamBase::Write(
    uv_buf_t* bufs,
    size_t count,
    uv_stream_t* send_handle,
    v8::Local<v8::Object> req_wrap_obj) {
  Environment* env = stream_env();
  int err;

  size_t total_bytes = 0;
  for (size_t i = 0; i < count; ++i)
    total_bytes += bufs[i].len;
  bytes_written_ += total_bytes;

  if (send_handle == nullptr) {
    err = DoTryWrite(&bufs, &count);
    if (err != 0 || count == 0) {
      return StreamWriteResult { false, err, nullptr, total_bytes };
    }
  if (send_handle == nullptr) {
    err = DoTryWrite(&bufs, &count);
    if (err != 0 || count == 0) {
      return StreamWriteResult { false, err, nullptr, total_bytes };
    }
  if (req_wrap_obj.IsEmpty()) {
    if (!env->write_wrap_template()
             ->NewInstance(env->context())
             .ToLocal(&req_wrap_obj)) {
      return StreamWriteResult { false, UV_EBUSY, nullptr, 0 };
    }
  if (msg != nullptr) {
    req_wrap_obj->Set(env->context(),
                      env->error_string(),
                      OneByteString(env->isolate(), msg)).Check();
    ClearError();
  }

  return StreamWriteResult { async, err, req_wrap, total_bytes };
}

template <typename OtherBase>
SimpleShutdownWrap<OtherBase>::SimpleShutdownWrap(
    StreamBase* stream,
    v8::Local<v8::Object> req_wrap_obj)
  : ShutdownWrap(stream, req_wrap_obj),
    OtherBase(stream->stream_env(),
              req_wrap_obj,
              AsyncWrap::PROVIDER_SHUTDOWNWRAP) {
}

template <typename OtherBase>
SimpleWriteWrap<OtherBase>::SimpleWriteWrap(
    StreamBase* stream,
    v8::Local<v8::Object> req_wrap_obj)
  : WriteWrap(stream, req_wrap_obj),
    OtherBase(stream->stream_env(),
              req_wrap_obj,
              AsyncWrap::PROVIDER_WRITEWRAP) {
}

void StreamBase::AttachToObject(v8::Local<v8::Object> obj) {
  obj->SetAlignedPointerInInternalField(
      StreamBase::kStreamBaseField, this);
}

StreamBase* StreamBase::FromObject(v8::Local<v8::Object> obj) {
  if (obj->GetAlignedPointerFromInternalField(StreamBase::kSlot) == nullptr)
    return nullptr;

  return static_cast<StreamBase*>(
      obj->GetAlignedPointerFromInternalField(
          StreamBase::kStreamBaseField));
}

WriteWrap* WriteWrap::FromObject(v8::Local<v8::Object> req_wrap_obj) {
  return static_cast<WriteWrap*>(StreamReq::FromObject(req_wrap_obj));
}

template <typename T, bool kIsWeak>
WriteWrap* WriteWrap::FromObject(
    const BaseObjectPtrImpl<T, kIsWeak>& base_obj) {
  if (!base_obj) return nullptr;
  return FromObject(base_obj->object());
}

ShutdownWrap* ShutdownWrap::FromObject(v8::Local<v8::Object> req_wrap_obj) {
  return static_cast<ShutdownWrap*>(StreamReq::FromObject(req_wrap_obj));
}

template <typename T, bool kIsWeak>
ShutdownWrap* ShutdownWrap::FromObject(
    const BaseObjectPtrImpl<T, kIsWeak>& base_obj) {
  if (!base_obj) return nullptr;
  return FromObject(base_obj->object());
}

void WriteWrap::SetAllocatedStorage(AllocatedBuffer&& storage) {
  CHECK_NULL(storage_.data());
  storage_ = std::move(storage);
}

void StreamReq::Done(int status, const char* error_str) {
  AsyncWrap* async_wrap = GetAsyncWrap();
  Environment* env = async_wrap->env();
  if (error_str != nullptr) {
    async_wrap->object()->Set(env->context(),
                              env->error_string(),
                              OneByteString(env->isolate(), error_str))
                              .Check();
  }

  OnDone(status);
}

void StreamReq::ResetObject(v8::Local<v8::Object> obj) {
  DCHECK_GT(obj->InternalFieldCount(), StreamReq::kStreamReqField);

  obj->SetAlignedPointerInInternalField(StreamReq::kSlot, nullptr);
  obj->SetAlignedPointerInInternalField(StreamReq::kStreamReqField, nullptr);
}

}  // namespace node

#endif  // defined(NODE_WANT_INTERNALS) && NODE_WANT_INTERNALS

#endif  // SRC_STREAM_BASE_INL_H_