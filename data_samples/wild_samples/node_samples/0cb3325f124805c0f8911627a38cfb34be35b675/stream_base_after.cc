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