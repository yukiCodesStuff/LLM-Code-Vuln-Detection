    return is_scope_granted(permission, res);
  }

  FORCE_INLINE bool enabled() const { return enabled_; }

  static PermissionScope StringToPermission(const std::string& perm);
  static const char* PermissionToString(PermissionScope perm);
  static void ThrowAccessDenied(Environment* env,
                                PermissionScope perm,