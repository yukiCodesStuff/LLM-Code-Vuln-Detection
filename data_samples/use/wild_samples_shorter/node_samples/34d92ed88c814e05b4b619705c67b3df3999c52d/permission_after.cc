      std::make_shared<ChildProcessPermission>();
  std::shared_ptr<PermissionBase> worker_t =
      std::make_shared<WorkerPermission>();
  std::shared_ptr<PermissionBase> inspector =
      std::make_shared<InspectorPermission>();
#define V(Name, _, __)                                                         \
  nodes_.insert(std::make_pair(PermissionScope::k##Name, fs));
  FILESYSTEM_PERMISSIONS(V)
#undef V
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