const kStats = Symbol('stats');
const assert = require('internal/assert');

const { encodeUtf8String } = internalBinding('encoding_binding');

const {
  fs: {
    F_OK = 0,
    W_OK = 0,
    }
    assert(isUint8Array(path));
    if (!BufferIsBuffer(path)) path = BufferFrom(path);
    // Avoid Buffer.from() and use a C++ binding instead to encode the result
    // of path.resolve() in order to prevent path traversal attacks that
    // monkey-patch Buffer internals.
    return encodeUtf8String(resolvePath(BufferToString(path)));
  }
  return path;
}
