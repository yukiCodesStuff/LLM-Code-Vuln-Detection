  if (r < 0) return Just(r);
  uv_process_options_.file = file_buffer_;

  Local<Value> js_args =
      js_options->Get(context, env()->args_string()).ToLocalChecked();
  if (!CopyJsStringArray(js_args, &args_buffer_).To(&r)) return Nothing<int>();
  if (r < 0) return Just(r);