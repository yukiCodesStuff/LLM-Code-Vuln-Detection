  if (nread <= 0)  {
    free(buf.base);
    if (nread < 0)
      stream->CallJSOnreadMethod(nread, Local<Object>());
    return;
  }

  CHECK_LE(static_cast<size_t>(nread), buf.len);
  char* base = Realloc(buf.base, nread);

  Local<Object> obj = Buffer::New(env, base, nread).ToLocalChecked();
  stream->CallJSOnreadMethod(nread, obj);
}


void ReportWritesToJSStreamListener::OnStreamAfterReqFinished(
    StreamReq* req_wrap, int status) {
  StreamBase* stream = static_cast<StreamBase*>(stream_);
  Environment* env = stream->stream_env();
  AsyncWrap* async_wrap = req_wrap->GetAsyncWrap();
  HandleScope handle_scope(env->isolate());
  Context::Scope context_scope(env->context());
  CHECK(!async_wrap->persistent().IsEmpty());
  Local<Object> req_wrap_obj = async_wrap->object();

  Local<Value> argv[] = {
    Integer::New(env->isolate(), status),
    stream->GetObject(),
    Undefined(env->isolate())
  };

  const char* msg = stream->Error();
  if (msg != nullptr) {
    argv[2] = OneByteString(env->isolate(), msg);
    stream->ClearError();
  }

  if (req_wrap_obj->Has(env->context(), env->oncomplete_string()).FromJust())
    async_wrap->MakeCallback(env->oncomplete_string(), arraysize(argv), argv);
}