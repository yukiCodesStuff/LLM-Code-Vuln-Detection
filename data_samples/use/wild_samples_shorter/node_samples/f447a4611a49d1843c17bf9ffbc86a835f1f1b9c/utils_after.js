  Symbol,
  TypedArrayPrototypeAt,
  TypedArrayPrototypeIncludes,
  uncurryThis,
} = primordials;

const permission = require('internal/process/permission');

// See: https://github.com/nodejs/node/pull/44004#discussion_r930958420
// The permission model needs the absolute path for the fs_permission
const resolvePath = pathModule.resolve;
const { isBuffer: BufferIsBuffer, from: BufferFrom } = Buffer;
const BufferToString = uncurryThis(Buffer.prototype.toString);
function possiblyTransformPath(path) {
  if (permission.isEnabled()) {
    if (typeof path === 'string') {
      return resolvePath(path);
    }
    assert(isUint8Array(path));
    if (!BufferIsBuffer(path)) path = BufferFrom(path);
    return BufferFrom(resolvePath(BufferToString(path)));
  }
  return path;
}
