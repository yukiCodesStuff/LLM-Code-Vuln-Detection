    ASSIGN_OR_RETURN_UNWRAP(&wrap, args.Holder());
    THROW_IF_INSUFFICIENT_PERMISSIONS(
        env, permission::PermissionScope::kChildProcess, "");

    Local<Object> js_options =
        args[0]->ToObject(env->context()).ToLocalChecked();

    node::Utf8Value file(env->isolate(), file_v);
    options.file = *file;

    // options.args
    Local<Value> argv_v =
        js_options->Get(context, env->args_string()).ToLocalChecked();
    if (!argv_v.IsEmpty() && argv_v->IsArray()) {
      options.flags |= UV_PROCESS_DETACHED;
    }

    int err = uv_spawn(env->event_loop(), &wrap->process_, &options);
    wrap->MarkAsInitialized();

    if (err == 0) {
      CHECK_EQ(wrap->process_.data, wrap);
      wrap->object()->Set(context, env->pid_string(),