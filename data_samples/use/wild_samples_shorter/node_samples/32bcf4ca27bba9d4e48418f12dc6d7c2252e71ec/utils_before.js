// TODO(rafaelgss): implement the path.resolve on C++ side
// See: https://github.com/nodejs/node/pull/44004#discussion_r930958420
// The permission model needs the absolute path for the fs_permission
function possiblyTransformPath(path) {
  if (permission.isEnabled()) {
    if (typeof path === 'string') {
      return pathModule.resolve(path);
    } else if (Buffer.isBuffer(path)) {
      return Buffer.from(pathModule.resolve(path.toString()));
    }
  }
  return path;
}