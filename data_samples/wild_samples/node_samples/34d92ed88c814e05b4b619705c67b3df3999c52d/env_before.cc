  if (options_->experimental_permission) {
    permission()->EnablePermissions();
    // If any permission is set the process shouldn't be able to neither
    // spawn/worker nor use addons unless explicitly allowed by the user
    if (!options_->allow_fs_read.empty() || !options_->allow_fs_write.empty()) {
      options_->allow_native_addons = false;
      if (!options_->allow_child_process) {
        permission()->Apply("*", permission::PermissionScope::kChildProcess);
      }
      if (!options_->allow_worker_threads) {
        permission()->Apply("*", permission::PermissionScope::kWorkerThreads);
      }
    }