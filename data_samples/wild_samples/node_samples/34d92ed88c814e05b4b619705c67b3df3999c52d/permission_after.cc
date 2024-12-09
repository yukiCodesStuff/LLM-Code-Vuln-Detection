Permission::Permission() : enabled_(false) {
  std::shared_ptr<PermissionBase> fs = std::make_shared<FSPermission>();
  std::shared_ptr<PermissionBase> child_p =
      std::make_shared<ChildProcessPermission>();
  std::shared_ptr<PermissionBase> worker_t =
      std::make_shared<WorkerPermission>();
  std::shared_ptr<PermissionBase> inspector =
      std::make_shared<InspectorPermission>();
#define V(Name, _, __)                                                         \
  nodes_.insert(std::make_pair(PermissionScope::k##Name, fs));
  FILESYSTEM_PERMISSIONS(V)
#undef V
#define V(Name, _, __)                                                         \
  nodes_.insert(std::make_pair(PermissionScope::k##Name, child_p));
  CHILD_PROCESS_PERMISSIONS(V)
#undef V
#define V(Name, _, __)                                                         \
  nodes_.insert(std::make_pair(PermissionScope::k##Name, worker_t));
  WORKER_THREADS_PERMISSIONS(V)
#undef V
#define V(Name, _, __)                                                         \
  nodes_.insert(std::make_pair(PermissionScope::k##Name, inspector));
  INSPECTOR_PERMISSIONS(V)
#undef V
}

void Permission::ThrowAccessDenied(Environment* env,
                                   PermissionScope perm,
                                   const std::string_view& res) {
  Local<Value> err = ERR_ACCESS_DENIED(env->isolate());
  CHECK(err->IsObject());
  if (err.As<Object>()
          ->Set(env->context(),
                env->permission_string(),
                v8::String::NewFromUtf8(env->isolate(),
                                        PermissionToString(perm),
                                        v8::NewStringType::kNormal)
                    .ToLocalChecked())
          .IsNothing() ||
      err.As<Object>()
          ->Set(env->context(),
                env->resource_string(),
                v8::String::NewFromUtf8(env->isolate(),
                                        std::string(res).c_str(),
                                        v8::NewStringType::kNormal)
                    .ToLocalChecked())
          .IsNothing())
    return;
  env->isolate()->ThrowException(err);
}

void Permission::EnablePermissions() {
  if (!enabled_) {
    enabled_ = true;
  }
Permission::Permission() : enabled_(false) {
  std::shared_ptr<PermissionBase> fs = std::make_shared<FSPermission>();
  std::shared_ptr<PermissionBase> child_p =
      std::make_shared<ChildProcessPermission>();
  std::shared_ptr<PermissionBase> worker_t =
      std::make_shared<WorkerPermission>();
  std::shared_ptr<PermissionBase> inspector =
      std::make_shared<InspectorPermission>();
#define V(Name, _, __)                                                         \
  nodes_.insert(std::make_pair(PermissionScope::k##Name, fs));
  FILESYSTEM_PERMISSIONS(V)
#undef V
#define V(Name, _, __)                                                         \
  nodes_.insert(std::make_pair(PermissionScope::k##Name, child_p));
  CHILD_PROCESS_PERMISSIONS(V)
#undef V
#define V(Name, _, __)                                                         \
  nodes_.insert(std::make_pair(PermissionScope::k##Name, worker_t));
  WORKER_THREADS_PERMISSIONS(V)
#undef V
#define V(Name, _, __)                                                         \
  nodes_.insert(std::make_pair(PermissionScope::k##Name, inspector));
  INSPECTOR_PERMISSIONS(V)
#undef V
}

void Permission::ThrowAccessDenied(Environment* env,
                                   PermissionScope perm,
                                   const std::string_view& res) {
  Local<Value> err = ERR_ACCESS_DENIED(env->isolate());
  CHECK(err->IsObject());
  if (err.As<Object>()
          ->Set(env->context(),
                env->permission_string(),
                v8::String::NewFromUtf8(env->isolate(),
                                        PermissionToString(perm),
                                        v8::NewStringType::kNormal)
                    .ToLocalChecked())
          .IsNothing() ||
      err.As<Object>()
          ->Set(env->context(),
                env->resource_string(),
                v8::String::NewFromUtf8(env->isolate(),
                                        std::string(res).c_str(),
                                        v8::NewStringType::kNormal)
                    .ToLocalChecked())
          .IsNothing())
    return;
  env->isolate()->ThrowException(err);
}

void Permission::EnablePermissions() {
  if (!enabled_) {
    enabled_ = true;
  }