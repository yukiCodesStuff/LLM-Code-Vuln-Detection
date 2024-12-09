// TODO(rafaelgss): implement the path.resolve on C++ side
// See: https://github.com/nodejs/node/pull/44004#discussion_r930958420
// The permission model needs the absolute path for the fs_permission
const resolvePath = pathModule.resolve;
function possiblyTransformPath(path) {
  if (permission.isEnabled()) {
    if (typeof path === 'string') {
      return resolvePath(path);
    } else if (Buffer.isBuffer(path)) {
      return Buffer.from(resolvePath(path.toString()));
    }
  }
  return path;
}