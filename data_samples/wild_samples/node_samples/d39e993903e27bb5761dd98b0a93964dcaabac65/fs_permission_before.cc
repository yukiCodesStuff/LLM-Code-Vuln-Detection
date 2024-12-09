    const std::string_view& param) {
  std::string resolved_param = node::PathResolve(env, {param});
#ifdef _WIN32
  // is UNC file path
  if (resolved_param.rfind("\\\\", 0) == 0) {
    // return lookup with normalized param
    size_t starting_pos = 4;  // "\\?\"
    if (resolved_param.rfind("\\\\?\\UNC\\") == 0) {
      starting_pos += 4;  // "UNC\"
    }
    auto normalized = param.substr(starting_pos);
    return granted_tree->Lookup(normalized, true);
  }