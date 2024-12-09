static void StatFs(const FunctionCallbackInfo<Value>& args) {
  BindingData* binding_data = Realm::GetBindingData<BindingData>(args);
  Environment* env = binding_data->env();

  const int argc = args.Length();
  CHECK_GE(argc, 2);

  BufferValue path(env->isolate(), args[0]);
  CHECK_NOT_NULL(*path);

  bool use_bigint = args[1]->IsTrue();
  FSReqBase* req_wrap_async = GetReqWrap(args, 2, use_bigint);
  if (req_wrap_async != nullptr) {  // statfs(path, use_bigint, req)
    FS_ASYNC_TRACE_BEGIN1(
        UV_FS_STATFS, req_wrap_async, "path", TRACE_STR_COPY(*path))
    AsyncCall(env,
              req_wrap_async,
              args,
              "statfs",
              UTF8,
              AfterStatFs,
              uv_fs_statfs,
              *path);
  } else {  // statfs(path, use_bigint, undefined, ctx)
    CHECK_EQ(argc, 4);
    FSReqWrapSync req_wrap_sync;
    FS_SYNC_TRACE_BEGIN(statfs);
    int err =
        SyncCall(env, args[3], &req_wrap_sync, "statfs", uv_fs_statfs, *path);
    FS_SYNC_TRACE_END(statfs);
    if (err != 0) {
      return;  // error info is in ctx
    }