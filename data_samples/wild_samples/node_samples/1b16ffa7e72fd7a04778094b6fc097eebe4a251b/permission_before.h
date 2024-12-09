                               const std::string_view& res = "") const {
    if (LIKELY(!enabled_)) return true;
    return is_scope_granted(permission, res);
  }

  static PermissionScope StringToPermission(const std::string& perm);
  static const char* PermissionToString(PermissionScope perm);
  static void ThrowAccessDenied(Environment* env,
                                PermissionScope perm,
                                const std::string_view& res);

  // CLI Call
  void Apply(const std::string& allow, PermissionScope scope);
  void EnablePermissions();

 private:
  COLD_NOINLINE bool is_scope_granted(const PermissionScope permission,
                                      const std::string_view& res = "") const {
    auto perm_node = nodes_.find(permission);
    if (perm_node != nodes_.end()) {
      return perm_node->second->is_granted(permission, res);
    }