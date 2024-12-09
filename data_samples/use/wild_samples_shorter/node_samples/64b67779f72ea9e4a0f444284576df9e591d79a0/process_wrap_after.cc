    ASSIGN_OR_RETURN_UNWRAP(&wrap, args.Holder());
    THROW_IF_INSUFFICIENT_PERMISSIONS(
        env, permission::PermissionScope::kChildProcess, "");
    int err = 0;

    Local<Object> js_options =
        args[0]->ToObject(env->context()).ToLocalChecked();

    node::Utf8Value file(env->isolate(), file_v);
    options.file = *file;

    // Undocumented feature of Win32 CreateProcess API allows spawning
    // batch files directly but is potentially insecure because arguments
    // are not escaped (and sometimes cannot be unambiguously escaped),
    // hence why they are rejected here.
    if (IsWindowsBatchFile(options.file))
      err = UV_EINVAL;

    // options.args
    Local<Value> argv_v =
        js_options->Get(context, env->args_string()).ToLocalChecked();
    if (!argv_v.IsEmpty() && argv_v->IsArray()) {
      options.flags |= UV_PROCESS_DETACHED;
    }

    if (err == 0) {
      err = uv_spawn(env->event_loop(), &wrap->process_, &options);
      wrap->MarkAsInitialized();
    }

    if (err == 0) {
      CHECK_EQ(wrap->process_.data, wrap);
      wrap->object()->Set(context, env->pid_string(),