// The permission model needs the absolute path for the fs_permission
function possiblyTransformPath(path) {
  if (permission.isEnabled()) {
    if (typeof path === 'string' && !pathModule.isAbsolute(path)) {
      return pathModule.resolve(path);
    }
  }
  return path;