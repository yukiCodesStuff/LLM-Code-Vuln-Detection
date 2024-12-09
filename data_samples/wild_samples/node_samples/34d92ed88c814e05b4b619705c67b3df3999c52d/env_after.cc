  if (options_->experimental_permission) {
    permission()->EnablePermissions();
    // If any permission is set the process shouldn't be able to neither
    // spawn/worker nor use addons or enable inspector
    // unless explicitly allowed by the user
    if (!options_->allow_fs_read.empty() || !options_->allow_fs_write.empty()) {
      options_->allow_native_addons = false;
      flags_ = flags_ | EnvironmentFlags::kNoCreateInspector;
      permission()->Apply("*", permission::PermissionScope::kInspector);
      if (!options_->allow_child_process) {
        permission()->Apply("*", permission::PermissionScope::kChildProcess);
      }
      if (!options_->allow_worker_threads) {
        permission()->Apply("*", permission::PermissionScope::kWorkerThreads);
      }
    }