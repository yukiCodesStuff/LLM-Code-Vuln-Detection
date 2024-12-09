  AsyncWrap* async_wrap = req_wrap->GetAsyncWrap();
  HandleScope handle_scope(env->isolate());
  Context::Scope context_scope(env->context());
  Local<Object> req_wrap_obj = async_wrap->object();

  Local<Value> argv[] = {
    Integer::New(env->isolate(), status),