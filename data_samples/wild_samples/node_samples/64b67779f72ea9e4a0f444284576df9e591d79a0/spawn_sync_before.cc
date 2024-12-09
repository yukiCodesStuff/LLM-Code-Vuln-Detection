Maybe<int> SyncProcessRunner::ParseOptions(Local<Value> js_value) {
  Isolate* isolate = env()->isolate();
  HandleScope scope(isolate);
  int r;

  if (!js_value->IsObject()) return Just<int>(UV_EINVAL);

  Local<Context> context = env()->context();
  Local<Object> js_options = js_value.As<Object>();

  Local<Value> js_file =
      js_options->Get(context, env()->file_string()).ToLocalChecked();
  if (!CopyJsString(js_file, &file_buffer_).To(&r)) return Nothing<int>();
  if (r < 0) return Just(r);
  uv_process_options_.file = file_buffer_;

  Local<Value> js_args =
      js_options->Get(context, env()->args_string()).ToLocalChecked();
  if (!CopyJsStringArray(js_args, &args_buffer_).To(&r)) return Nothing<int>();
  if (r < 0) return Just(r);
  uv_process_options_.args = reinterpret_cast<char**>(args_buffer_);

  Local<Value> js_cwd =
      js_options->Get(context, env()->cwd_string()).ToLocalChecked();
  if (IsSet(js_cwd)) {
    if (!CopyJsString(js_cwd, &cwd_buffer_).To(&r)) return Nothing<int>();
    if (r < 0) return Just(r);
    uv_process_options_.cwd = cwd_buffer_;
  }