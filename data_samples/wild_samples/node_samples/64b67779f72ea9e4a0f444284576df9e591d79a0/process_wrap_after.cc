  static void Spawn(const FunctionCallbackInfo<Value>& args) {
    Environment* env = Environment::GetCurrent(args);
    Local<Context> context = env->context();
    ProcessWrap* wrap;
    ASSIGN_OR_RETURN_UNWRAP(&wrap, args.Holder());
    THROW_IF_INSUFFICIENT_PERMISSIONS(
        env, permission::PermissionScope::kChildProcess, "");
    int err = 0;

    Local<Object> js_options =
        args[0]->ToObject(env->context()).ToLocalChecked();

    uv_process_options_t options;
    memset(&options, 0, sizeof(uv_process_options_t));

    options.exit_cb = OnExit;

    // options.uid
    Local<Value> uid_v =
        js_options->Get(context, env->uid_string()).ToLocalChecked();
    if (!uid_v->IsUndefined() && !uid_v->IsNull()) {
      CHECK(uid_v->IsInt32());
      const int32_t uid = uid_v.As<Int32>()->Value();
      options.flags |= UV_PROCESS_SETUID;
      options.uid = static_cast<uv_uid_t>(uid);
    }
    if (!gid_v->IsUndefined() && !gid_v->IsNull()) {
      CHECK(gid_v->IsInt32());
      const int32_t gid = gid_v.As<Int32>()->Value();
      options.flags |= UV_PROCESS_SETGID;
      options.gid = static_cast<uv_gid_t>(gid);
    }

    // TODO(bnoordhuis) is this possible to do without mallocing ?

    // options.file
    Local<Value> file_v =
        js_options->Get(context, env->file_string()).ToLocalChecked();
    CHECK(file_v->IsString());
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
      Local<Array> js_argv = argv_v.As<Array>();
      int argc = js_argv->Length();
      CHECK_LT(argc, INT_MAX);  // Check for overflow.

      // Heap allocate to detect errors. +1 is for nullptr.
      options.args = new char*[argc + 1];
      for (int i = 0; i < argc; i++) {
        node::Utf8Value arg(env->isolate(),
                            js_argv->Get(context, i).ToLocalChecked());
        options.args[i] = strdup(*arg);
        CHECK_NOT_NULL(options.args[i]);
      }
      options.args[argc] = nullptr;
    }
    if (detached_v->IsTrue()) {
      options.flags |= UV_PROCESS_DETACHED;
    }

    if (err == 0) {
      err = uv_spawn(env->event_loop(), &wrap->process_, &options);
      wrap->MarkAsInitialized();
    }

    if (err == 0) {
      CHECK_EQ(wrap->process_.data, wrap);
      wrap->object()->Set(context, env->pid_string(),
                          Integer::New(env->isolate(),
                                       wrap->process_.pid)).Check();
    }

    if (options.args) {
      for (int i = 0; options.args[i]; i++) free(options.args[i]);
      delete [] options.args;
    }

    if (options.env) {
      for (int i = 0; options.env[i]; i++) free(options.env[i]);
      delete [] options.env;
    }

    delete[] options.stdio;

    args.GetReturnValue().Set(err);
  }

  static void Kill(const FunctionCallbackInfo<Value>& args) {
    Environment* env = Environment::GetCurrent(args);
    ProcessWrap* wrap;
    ASSIGN_OR_RETURN_UNWRAP(&wrap, args.Holder());
    int signal = args[0]->Int32Value(env->context()).FromJust();
    int err = uv_process_kill(&wrap->process_, signal);
    args.GetReturnValue().Set(err);
  }

  static void OnExit(uv_process_t* handle,
                     int64_t exit_status,
                     int term_signal) {
    ProcessWrap* wrap = ContainerOf(&ProcessWrap::process_, handle);
    CHECK_EQ(&wrap->process_, handle);

    Environment* env = wrap->env();
    HandleScope handle_scope(env->isolate());
    Context::Scope context_scope(env->context());

    Local<Value> argv[] = {
      Number::New(env->isolate(), static_cast<double>(exit_status)),
      OneByteString(env->isolate(), signo_string(term_signal))
    };

    wrap->MakeCallback(env->onexit_string(), arraysize(argv), argv);
  }

  uv_process_t process_;
};


}  // anonymous namespace
}  // namespace node

NODE_BINDING_CONTEXT_AWARE_INTERNAL(process_wrap, node::ProcessWrap::Initialize)
NODE_BINDING_EXTERNAL_REFERENCE(process_wrap,
                                node::ProcessWrap::RegisterExternalReferences)