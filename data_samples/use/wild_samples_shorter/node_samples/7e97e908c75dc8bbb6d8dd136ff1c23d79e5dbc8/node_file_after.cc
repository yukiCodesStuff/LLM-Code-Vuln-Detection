
  BufferValue path(env->isolate(), args[0]);
  CHECK_NOT_NULL(*path);
  THROW_IF_INSUFFICIENT_PERMISSIONS(
      env, permission::PermissionScope::kFileSystemRead, path.ToStringView());

  bool use_bigint = args[1]->IsTrue();
  FSReqBase* req_wrap_async = GetReqWrap(args, 2, use_bigint);
  if (req_wrap_async != nullptr) {  // statfs(path, use_bigint, req)