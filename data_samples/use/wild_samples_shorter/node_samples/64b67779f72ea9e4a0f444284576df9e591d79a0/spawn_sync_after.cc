  if (r < 0) return Just(r);
  uv_process_options_.file = file_buffer_;

  // Undocumented feature of Win32 CreateProcess API allows spawning
  // batch files directly but is potentially insecure because arguments
  // are not escaped (and sometimes cannot be unambiguously escaped),
  // hence why they are rejected here.
  if (IsWindowsBatchFile(uv_process_options_.file))
    return Just<int>(UV_EINVAL);

  Local<Value> js_args =
      js_options->Get(context, env()->args_string()).ToLocalChecked();
  if (!CopyJsStringArray(js_args, &args_buffer_).To(&r)) return Nothing<int>();
  if (r < 0) return Just(r);