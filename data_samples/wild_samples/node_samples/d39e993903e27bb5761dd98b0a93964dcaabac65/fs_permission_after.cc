    const std::string_view& param) {
  std::string resolved_param = node::PathResolve(env, {param});
#ifdef _WIN32
  // Remove leading "\\?\" from UNC path
  if (resolved_param.substr(0, 4) == "\\\\?\\") {
    resolved_param.erase(0, 4);
  }