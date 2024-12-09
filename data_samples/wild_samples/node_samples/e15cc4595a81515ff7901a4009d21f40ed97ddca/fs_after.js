function openAsBlob(path, options = kEmptyObject) {
  validateObject(options, 'options');
  const type = options.type || '';
  validateString(type, 'options.type');
  // The underlying implementation here returns the Blob synchronously for now.
  // To give ourselves flexibility to maybe return the Blob asynchronously,
  // this API returns a Promise.
  path = getValidatedPath(path);
  return PromiseResolve(createBlobFromFilePath(pathModule.toNamespacedPath(path), { type }));
}

/**
 * Reads file from the specified `fd` (file descriptor).
 * @param {number} fd
 * @param {Buffer | TypedArray | DataView} buffer
 * @param {number} offsetOrOptions
 * @param {number} length
 * @param {number | bigint | null} position
 * @param {(
 *   err?: Error,
 *   bytesRead?: number,
 *   buffer?: Buffer
 *   ) => any} callback
 * @returns {void}
 */
function read(fd, buffer, offsetOrOptions, length, position, callback) {
  fd = getValidatedFd(fd);

  let offset = offsetOrOptions;
  let params = null;
  if (arguments.length <= 4) {
    if (arguments.length === 4) {
      // This is fs.read(fd, buffer, options, callback)
      validateObject(offsetOrOptions, 'options', { nullable: true });
      callback = length;
      params = offsetOrOptions;
    } else if (arguments.length === 3) {
      // This is fs.read(fd, bufferOrParams, callback)
      if (!isArrayBufferView(buffer)) {
        // This is fs.read(fd, params, callback)
        params = buffer;
        ({ buffer = Buffer.alloc(16384) } = params ?? kEmptyObject);
      }
      callback = offsetOrOptions;
    } else {
      // This is fs.read(fd, callback)
      callback = buffer;
      buffer = Buffer.alloc(16384);
    }

    if (params !== undefined) {
      validateObject(params, 'options', { nullable: true });
    }
    ({
      offset = 0,
      length = buffer?.byteLength - offset,
      position = null,
    } = params ?? kEmptyObject);
  }

  validateBuffer(buffer);
  callback = maybeCallback(callback);

  if (offset == null) {
    offset = 0;
  } else {
    validateInteger(offset, 'offset', 0);
  }

  length |= 0;

  if (length === 0) {
    return process.nextTick(function tick() {
      callback(null, 0, buffer);
    });
  }

  if (buffer.byteLength === 0) {
    throw new ERR_INVALID_ARG_VALUE('buffer', buffer,
                                    'is empty and cannot be written');
  }

  validateOffsetLengthRead(offset, length, buffer.byteLength);

  if (position == null)
    position = -1;

  validatePosition(position, 'position');

  function wrapper(err, bytesRead) {
    // Retain a reference to buffer so that it can't be GC'ed too soon.
    callback(err, bytesRead || 0, buffer);
  }

  const req = new FSReqCallback();
  req.oncomplete = wrapper;

  binding.read(fd, buffer, offset, length, position, req);
}

ObjectDefineProperty(read, kCustomPromisifyArgsSymbol,
                     { __proto__: null, value: ['bytesRead', 'buffer'], enumerable: false });

/**
 * Synchronously reads the file from the
 * specified `fd` (file descriptor).
 * @param {number} fd
 * @param {Buffer | TypedArray | DataView} buffer
 * @param {{
 *   offset?: number;
 *   length?: number;
 *   position?: number | bigint | null;
 *   }} [offsetOrOptions]
 * @returns {number}
 */
function readSync(fd, buffer, offsetOrOptions, length, position) {
  fd = getValidatedFd(fd);

  validateBuffer(buffer);

  let offset = offsetOrOptions;
  if (arguments.length <= 3 || typeof offsetOrOptions === 'object') {
    if (offsetOrOptions !== undefined) {
      validateObject(offsetOrOptions, 'options', { nullable: true });
    }

    ({
      offset = 0,
      length = buffer.byteLength - offset,
      position = null,
    } = offsetOrOptions ?? kEmptyObject);
  }

  if (offset === undefined) {
    offset = 0;
  } else {
    validateInteger(offset, 'offset', 0);
  }

  length |= 0;

  if (length === 0) {
    return 0;
  }

  if (buffer.byteLength === 0) {
    throw new ERR_INVALID_ARG_VALUE('buffer', buffer,
                                    'is empty and cannot be written');
  }

  validateOffsetLengthRead(offset, length, buffer.byteLength);

  if (position == null)
    position = -1;

  validatePosition(position, 'position');

  const ctx = {};
  const result = binding.read(fd, buffer, offset, length, position,
                              undefined, ctx);
  handleErrorFromBinding(ctx);
  return result;
}

/**
 * Reads file from the specified `fd` (file descriptor)
 * and writes to an array of `ArrayBufferView`s.
 * @param {number} fd
 * @param {ArrayBufferView[]} buffers
 * @param {number | null} [position]
 * @param {(
 *   err?: Error,
 *   bytesRead?: number,
 *   buffers?: ArrayBufferView[];
 *   ) => any} callback
 * @returns {void}
 */
function readv(fd, buffers, position, callback) {
  function wrapper(err, read) {
    callback(err, read || 0, buffers);
  }

  fd = getValidatedFd(fd);
  validateBufferArray(buffers);
  callback = maybeCallback(callback || position);

  const req = new FSReqCallback();
  req.oncomplete = wrapper;

  if (typeof position !== 'number')
    position = null;

  return binding.readBuffers(fd, buffers, position, req);
}

ObjectDefineProperty(readv, kCustomPromisifyArgsSymbol,
                     { __proto__: null, value: ['bytesRead', 'buffers'], enumerable: false });

/**
 * Synchronously reads file from the
 * specified `fd` (file descriptor) and writes to an array
 * of `ArrayBufferView`s.
 * @param {number} fd
 * @param {ArrayBufferView[]} buffers
 * @param {number | null} [position]
 * @returns {number}
 */
function readvSync(fd, buffers, position) {
  fd = getValidatedFd(fd);
  validateBufferArray(buffers);

  const ctx = {};

  if (typeof position !== 'number')
    position = null;

  const result = binding.readBuffers(fd, buffers, position, undefined, ctx);
  handleErrorFromBinding(ctx);
  return result;
}

/**
 * Writes `buffer` to the specified `fd` (file descriptor).
 * @param {number} fd
 * @param {Buffer | TypedArray | DataView | string} buffer
 * @param {number | object} [offsetOrOptions]
 * @param {number} [length]
 * @param {number | null} [position]
 * @param {(
 *   err?: Error,
 *   bytesWritten?: number;
 *   buffer?: Buffer | TypedArray | DataView
 *   ) => any} callback
 * @returns {void}
 */
function write(fd, buffer, offsetOrOptions, length, position, callback) {
  function wrapper(err, written) {
    // Retain a reference to buffer so that it can't be GC'ed too soon.
    callback(err, written || 0, buffer);
  }

  fd = getValidatedFd(fd);

  let offset = offsetOrOptions;
  if (isArrayBufferView(buffer)) {
    callback = maybeCallback(callback || position || length || offset);

    if (typeof offset === 'object') {
      ({
        offset = 0,
        length = buffer.byteLength - offset,
        position = null,
      } = offsetOrOptions ?? kEmptyObject);
    }

    if (offset == null || typeof offset === 'function') {
      offset = 0;
    } else {
      validateInteger(offset, 'offset', 0);
    }
    if (typeof length !== 'number')
      length = buffer.byteLength - offset;
    if (typeof position !== 'number')
      position = null;
    validateOffsetLengthWrite(offset, length, buffer.byteLength);

    const req = new FSReqCallback();
    req.oncomplete = wrapper;
    return binding.writeBuffer(fd, buffer, offset, length, position, req);
  }

  validateStringAfterArrayBufferView(buffer, 'buffer');

  if (typeof position !== 'function') {
    if (typeof offset === 'function') {
      position = offset;
      offset = null;
    } else {
      position = length;
    }
    length = 'utf8';
  }

  const str = buffer;
  validateEncoding(str, length);
  callback = maybeCallback(position);

  const req = new FSReqCallback();
  req.oncomplete = wrapper;
  return binding.writeString(fd, str, offset, length, req);
}

ObjectDefineProperty(write, kCustomPromisifyArgsSymbol,
                     { __proto__: null, value: ['bytesWritten', 'buffer'], enumerable: false });

/**
 * Synchronously writes `buffer` to the
 * specified `fd` (file descriptor).
 * @param {number} fd
 * @param {Buffer | TypedArray | DataView | string} buffer
 * @param {{
 *   offset?: number;
 *   length?: number;
 *   position?: number | null;
 *   }} [offsetOrOptions]
 * @returns {number}
 */
function writeSync(fd, buffer, offsetOrOptions, length, position) {
  fd = getValidatedFd(fd);
  const ctx = {};
  let result;

  let offset = offsetOrOptions;
  if (isArrayBufferView(buffer)) {
    if (typeof offset === 'object') {
      ({
        offset = 0,
        length = buffer.byteLength - offset,
        position = null,
      } = offsetOrOptions ?? kEmptyObject);
    }
    if (position === undefined)
      position = null;
    if (offset == null) {
      offset = 0;
    } else {
      validateInteger(offset, 'offset', 0);
    }
    if (typeof length !== 'number')
      length = buffer.byteLength - offset;
    validateOffsetLengthWrite(offset, length, buffer.byteLength);
    result = binding.writeBuffer(fd, buffer, offset, length, position,
                                 undefined, ctx);
  } else {
    validateStringAfterArrayBufferView(buffer, 'buffer');
    validateEncoding(buffer, length);

    if (offset === undefined)
      offset = null;
    result = binding.writeString(fd, buffer, offset, length,
                                 undefined, ctx);
  }
  handleErrorFromBinding(ctx);
  return result;
}

/**
 * Writes an array of `ArrayBufferView`s to the
 * specified `fd` (file descriptor).
 * @param {number} fd
 * @param {ArrayBufferView[]} buffers
 * @param {number | null} [position]
 * @param {(
 *   err?: Error,
 *   bytesWritten?: number,
 *   buffers?: ArrayBufferView[]
 *   ) => any} callback
 * @returns {void}
 */
function writev(fd, buffers, position, callback) {
  function wrapper(err, written) {
    callback(err, written || 0, buffers);
  }

  fd = getValidatedFd(fd);
  validateBufferArray(buffers);
  callback = maybeCallback(callback || position);

  if (buffers.length === 0) {
    process.nextTick(callback, null, 0, buffers);
    return;
  }

  const req = new FSReqCallback();
  req.oncomplete = wrapper;

  if (typeof position !== 'number')
    position = null;

  return binding.writeBuffers(fd, buffers, position, req);
}

ObjectDefineProperty(writev, kCustomPromisifyArgsSymbol, {
  __proto__: null,
  value: ['bytesWritten', 'buffer'],
  enumerable: false,
});

/**
 * Synchronously writes an array of `ArrayBufferView`s
 * to the specified `fd` (file descriptor).
 * @param {number} fd
 * @param {ArrayBufferView[]} buffers
 * @param {number | null} [position]
 * @returns {number}
 */
function writevSync(fd, buffers, position) {
  fd = getValidatedFd(fd);
  validateBufferArray(buffers);

  if (buffers.length === 0) {
    return 0;
  }

  const ctx = {};

  if (typeof position !== 'number')
    position = null;

  const result = binding.writeBuffers(fd, buffers, position, undefined, ctx);

  handleErrorFromBinding(ctx);
  return result;
}

/**
 * Asynchronously renames file at `oldPath` to
 * the pathname provided as `newPath`.
 * @param {string | Buffer | URL} oldPath
 * @param {string | Buffer | URL} newPath
 * @param {(err?: Error) => any} callback
 * @returns {void}
 */
function rename(oldPath, newPath, callback) {
  callback = makeCallback(callback);
  oldPath = getValidatedPath(oldPath, 'oldPath');
  newPath = getValidatedPath(newPath, 'newPath');
  const req = new FSReqCallback();
  req.oncomplete = callback;
  binding.rename(pathModule.toNamespacedPath(oldPath),
                 pathModule.toNamespacedPath(newPath),
                 req);
}


/**
 * Synchronously renames file at `oldPath` to
 * the pathname provided as `newPath`.
 * @param {string | Buffer | URL} oldPath
 * @param {string | Buffer | URL} newPath
 * @returns {void}
 */
function renameSync(oldPath, newPath) {
  oldPath = getValidatedPath(oldPath, 'oldPath');
  newPath = getValidatedPath(newPath, 'newPath');
  const ctx = { path: oldPath, dest: newPath };
  binding.rename(pathModule.toNamespacedPath(oldPath),
                 pathModule.toNamespacedPath(newPath), undefined, ctx);
  handleErrorFromBinding(ctx);
}

/**
 * Truncates the file.
 * @param {string | Buffer | URL} path
 * @param {number} [len]
 * @param {(err?: Error) => any} callback
 * @returns {void}
 */
function truncate(path, len, callback) {
  if (typeof path === 'number') {
    showTruncateDeprecation();
    return fs.ftruncate(path, len, callback);
  }
  if (typeof len === 'function') {
    callback = len;
    len = 0;
  } else if (len === undefined) {
    len = 0;
  }

  validateInteger(len, 'len');
  len = MathMax(0, len);
  callback = maybeCallback(callback);
  fs.open(path, 'r+', (er, fd) => {
    if (er) return callback(er);
    const req = new FSReqCallback();
    req.oncomplete = function oncomplete(er) {
      fs.close(fd, (er2) => {
        callback(aggregateTwoErrors(er2, er));
      });
    };
    binding.ftruncate(fd, len, req);
  });
}

/**
 * Synchronously truncates the file.
 * @param {string | Buffer | URL} path
 * @param {number} [len]
 * @returns {void}
 */
function truncateSync(path, len) {
  if (typeof path === 'number') {
    // legacy
    showTruncateDeprecation();
    return fs.ftruncateSync(path, len);
  }
  if (len === undefined) {
    len = 0;
  }
  // Allow error to be thrown, but still close fd.
  const fd = fs.openSync(path, 'r+');
  let ret;

  try {
    ret = fs.ftruncateSync(fd, len);
  } finally {
    fs.closeSync(fd);
  }
  return ret;
}

/**
 * Truncates the file descriptor.
 * @param {number} fd
 * @param {number} [len]
 * @param {(err?: Error) => any} callback
 * @returns {void}
 */
function ftruncate(fd, len = 0, callback) {
  if (typeof len === 'function') {
    callback = len;
    len = 0;
  }
  fd = getValidatedFd(fd);
  validateInteger(len, 'len');
  len = MathMax(0, len);
  callback = makeCallback(callback);

  const req = new FSReqCallback();
  req.oncomplete = callback;
  binding.ftruncate(fd, len, req);
}

/**
 * Synchronously truncates the file descriptor.
 * @param {number} fd
 * @param {number} [len]
 * @returns {void}
 */
function ftruncateSync(fd, len = 0) {
  fd = getValidatedFd(fd);
  validateInteger(len, 'len');
  len = MathMax(0, len);
  const ctx = {};
  binding.ftruncate(fd, len, undefined, ctx);
  handleErrorFromBinding(ctx);
}

function lazyLoadCp() {
  if (cpFn === undefined) {
    ({ cpFn } = require('internal/fs/cp/cp'));
    cpFn = require('util').callbackify(cpFn);
    ({ cpSyncFn } = require('internal/fs/cp/cp-sync'));
  }
}

function lazyLoadRimraf() {
  if (rimraf === undefined)
    ({ rimraf, rimrafSync } = require('internal/fs/rimraf'));
}

/**
 * Asynchronously removes a directory.
 * @param {string | Buffer | URL} path
 * @param {{
 *   maxRetries?: number;
 *   recursive?: boolean;
 *   retryDelay?: number;
 *   }} [options]
 * @param {(err?: Error) => any} callback
 * @returns {void}
 */
function rmdir(path, options, callback) {
  if (typeof options === 'function') {
    callback = options;
    options = undefined;
  }

  callback = makeCallback(callback);
  path = pathModule.toNamespacedPath(getValidatedPath(path));

  if (options?.recursive) {
    emitRecursiveRmdirWarning();
    validateRmOptions(
      path,
      { ...options, force: false },
      true,
      (err, options) => {
        if (err === false) {
          const req = new FSReqCallback();
          req.oncomplete = callback;
          return binding.rmdir(path, req);
        }
        if (err) {
          return callback(err);
        }

        lazyLoadRimraf();
        rimraf(path, options, callback);
      });
  } else {
    validateRmdirOptions(options);
    const req = new FSReqCallback();
    req.oncomplete = callback;
    return binding.rmdir(path, req);
  }
}

/**
 * Synchronously removes a directory.
 * @param {string | Buffer | URL} path
 * @param {{
 *   maxRetries?: number;
 *   recursive?: boolean;
 *   retryDelay?: number;
 *   }} [options]
 * @returns {void}
 */
function rmdirSync(path, options) {
  path = getValidatedPath(path);

  if (options?.recursive) {
    emitRecursiveRmdirWarning();
    options = validateRmOptionsSync(path, { ...options, force: false }, true);
    if (options !== false) {
      lazyLoadRimraf();
      return rimrafSync(pathModule.toNamespacedPath(path), options);
    }
  } else {
    validateRmdirOptions(options);
  }

  const ctx = { path };
  binding.rmdir(pathModule.toNamespacedPath(path), undefined, ctx);
  return handleErrorFromBinding(ctx);
}

/**
 * Asynchronously removes files and
 * directories (modeled on the standard POSIX `rm` utility).
 * @param {string | Buffer | URL} path
 * @param {{
 *   force?: boolean;
 *   maxRetries?: number;
 *   recursive?: boolean;
 *   retryDelay?: number;
 *   }} [options]
 * @param {(err?: Error) => any} callback
 * @returns {void}
 */
function rm(path, options, callback) {
  if (typeof options === 'function') {
    callback = options;
    options = undefined;
  }
  path = getValidatedPath(path);

  validateRmOptions(path, options, false, (err, options) => {
    if (err) {
      return callback(err);
    }
    lazyLoadRimraf();
    return rimraf(pathModule.toNamespacedPath(path), options, callback);
  });
}

/**
 * Synchronously removes files and
 * directories (modeled on the standard POSIX `rm` utility).
 * @param {string | Buffer | URL} path
 * @param {{
 *   force?: boolean;
 *   maxRetries?: number;
 *   recursive?: boolean;
 *   retryDelay?: number;
 *   }} [options]
 * @returns {void}
 */
function rmSync(path, options) {
  path = getValidatedPath(path);
  options = validateRmOptionsSync(path, options, false);

  lazyLoadRimraf();
  return rimrafSync(pathModule.toNamespacedPath(path), options);
}

/**
 * Forces all currently queued I/O operations associated
 * with the file to the operating system's synchronized
 * I/O completion state.
 * @param {number} fd
 * @param {(err?: Error) => any} callback
 * @returns {void}
 */
function fdatasync(fd, callback) {
  fd = getValidatedFd(fd);
  const req = new FSReqCallback();
  req.oncomplete = makeCallback(callback);
  binding.fdatasync(fd, req);
}

/**
 * Synchronously forces all currently queued I/O operations
 * associated with the file to the operating
 * system's synchronized I/O completion state.
 * @param {number} fd
 * @returns {void}
 */
function fdatasyncSync(fd) {
  fd = getValidatedFd(fd);
  const ctx = {};
  binding.fdatasync(fd, undefined, ctx);
  handleErrorFromBinding(ctx);
}

/**
 * Requests for all data for the open file descriptor
 * to be flushed to the storage device.
 * @param {number} fd
 * @param {(err?: Error) => any} callback
 * @returns {void}
 */
function fsync(fd, callback) {
  fd = getValidatedFd(fd);
  const req = new FSReqCallback();
  req.oncomplete = makeCallback(callback);
  binding.fsync(fd, req);
}

/**
 * Synchronously requests for all data for the open
 * file descriptor to be flushed to the storage device.
 * @param {number} fd
 * @returns {void}
 */
function fsyncSync(fd) {
  fd = getValidatedFd(fd);
  const ctx = {};
  binding.fsync(fd, undefined, ctx);
  handleErrorFromBinding(ctx);
}

/**
 * Asynchronously creates a directory.
 * @param {string | Buffer | URL} path
 * @param {{
 *   recursive?: boolean;
 *   mode?: string | number;
 *   } | number} [options]
 * @param {(err?: Error) => any} callback
 * @returns {void}
 */
function mkdir(path, options, callback) {
  let mode = 0o777;
  let recursive = false;
  if (typeof options === 'function') {
    callback = options;
  } else if (typeof options === 'number' || typeof options === 'string') {
    mode = options;
  } else if (options) {
    if (options.recursive !== undefined)
      recursive = options.recursive;
    if (options.mode !== undefined)
      mode = options.mode;
  }
  callback = makeCallback(callback);
  path = getValidatedPath(path);

  validateBoolean(recursive, 'options.recursive');

  const req = new FSReqCallback();
  req.oncomplete = callback;
  binding.mkdir(pathModule.toNamespacedPath(path),
                parseFileMode(mode, 'mode'), recursive, req);
}

/**
 * Synchronously creates a directory.
 * @param {string | Buffer | URL} path
 * @param {{
 *   recursive?: boolean;
 *   mode?: string | number;
 *   } | number} [options]
 * @returns {string | void}
 */
function mkdirSync(path, options) {
  let mode = 0o777;
  let recursive = false;
  if (typeof options === 'number' || typeof options === 'string') {
    mode = options;
  } else if (options) {
    if (options.recursive !== undefined)
      recursive = options.recursive;
    if (options.mode !== undefined)
      mode = options.mode;
  }
  path = getValidatedPath(path);
  validateBoolean(recursive, 'options.recursive');

  const ctx = { path };
  const result = binding.mkdir(pathModule.toNamespacedPath(path),
                               parseFileMode(mode, 'mode'), recursive,
                               undefined, ctx);
  handleErrorFromBinding(ctx);
  if (recursive) {
    return result;
  }
}

/**
 * An iterative algorithm for reading the entire contents of the `basePath` directory.
 * This function does not validate `basePath` as a directory. It is passed directly to
 * `binding.readdir` after a `nullCheck`.
 * @param {string} basePath
 * @param {{ encoding: string, withFileTypes: boolean }} options
 * @returns {string[] | Dirent[]}
 */
function readdirSyncRecursive(basePath, options) {
  nullCheck(basePath, 'path', true);

  const withFileTypes = Boolean(options.withFileTypes);
  const encoding = options.encoding;

  const readdirResults = [];
  const pathsQueue = [basePath];

  const ctx = { path: basePath };
  function read(path) {
    ctx.path = path;
    const readdirResult = binding.readdir(
      pathModule.toNamespacedPath(path),
      encoding,
      withFileTypes,
      undefined,
      ctx,
    );
    handleErrorFromBinding(ctx);

    for (let i = 0; i < readdirResult.length; i++) {
      if (withFileTypes) {
        const dirent = getDirent(path, readdirResult[0][i], readdirResult[1][i]);
        ArrayPrototypePush(readdirResults, dirent);
        if (dirent.isDirectory()) {
          ArrayPrototypePush(pathsQueue, pathModule.join(dirent.path, dirent.name));
        }
      } else {
        const resultPath = pathModule.join(path, readdirResult[i]);
        const relativeResultPath = pathModule.relative(basePath, resultPath);
        const stat = binding.internalModuleStat(resultPath);
        ArrayPrototypePush(readdirResults, relativeResultPath);
        // 1 indicates directory
        if (stat === 1) {
          ArrayPrototypePush(pathsQueue, resultPath);
        }
      }
    }
  }

  for (let i = 0; i < pathsQueue.length; i++) {
    read(pathsQueue[i]);
  }

  return readdirResults;
}

/**
 * Reads the contents of a directory.
 * @param {string | Buffer | URL} path
 * @param {string | {
 *   encoding?: string;
 *   withFileTypes?: boolean;
 *   }} [options]
 * @param {(
 *   err?: Error,
 *   files?: string[] | Buffer[] | Direct[];
 *   ) => any} callback
 * @returns {void}
 */
function readdir(path, options, callback) {
  callback = makeCallback(typeof options === 'function' ? options : callback);
  options = getOptions(options);
  path = getValidatedPath(path);
  if (options.recursive != null) {
    validateBoolean(options.recursive, 'options.recursive');
  }

  if (options.recursive) {
    callback(null, readdirSyncRecursive(path, options));
    return;
  }

  const req = new FSReqCallback();
  if (!options.withFileTypes) {
    req.oncomplete = callback;
  } else {
    req.oncomplete = (err, result) => {
      if (err) {
        callback(err);
        return;
      }
      getDirents(path, result, callback);
    };
  }
  binding.readdir(pathModule.toNamespacedPath(path), options.encoding,
                  !!options.withFileTypes, req);
}

/**
 * Synchronously reads the contents of a directory.
 * @param {string | Buffer | URL} path
 * @param {string | {
 *   encoding?: string;
 *   withFileTypes?: boolean;
 *   recursive?: boolean;
 *   }} [options]
 * @returns {string | Buffer[] | Dirent[]}
 */
function readdirSync(path, options) {
  options = getOptions(options);
  path = getValidatedPath(path);
  if (options.recursive != null) {
    validateBoolean(options.recursive, 'options.recursive');
  }

  if (options.recursive) {
    return readdirSyncRecursive(path, options);
  }

  const ctx = { path };
  const result = binding.readdir(pathModule.toNamespacedPath(path),
                                 options.encoding, !!options.withFileTypes,
                                 undefined, ctx);
  handleErrorFromBinding(ctx);
  return options.withFileTypes ? getDirents(path, result) : result;
}

/**
 * Invokes the callback with the `fs.Stats`
 * for the file descriptor.
 * @param {number} fd
 * @param {{ bigint?: boolean; }} [options]
 * @param {(
 *   err?: Error,
 *   stats?: Stats
 *   ) => any} callback
 * @returns {void}
 */
function fstat(fd, options = { bigint: false }, callback) {
  if (typeof options === 'function') {
    callback = options;
    options = kEmptyObject;
  }
  fd = getValidatedFd(fd);
  callback = makeStatsCallback(callback);

  const req = new FSReqCallback(options.bigint);
  req.oncomplete = callback;
  binding.fstat(fd, options.bigint, req);
}

/**
 * Retrieves the `fs.Stats` for the symbolic link
 * referred to by the `path`.
 * @param {string | Buffer | URL} path
 * @param {{ bigint?: boolean; }} [options]
 * @param {(
 *   err?: Error,
 *   stats?: Stats
 *   ) => any} callback
 * @returns {void}
 */
function lstat(path, options = { bigint: false }, callback) {
  if (typeof options === 'function') {
    callback = options;
    options = kEmptyObject;
  }
  callback = makeStatsCallback(callback);
  path = getValidatedPath(path);

  const req = new FSReqCallback(options.bigint);
  req.oncomplete = callback;
  binding.lstat(pathModule.toNamespacedPath(path), options.bigint, req);
}

/**
 * Asynchronously gets the stats of a file.
 * @param {string | Buffer | URL} path
 * @param {{ bigint?: boolean; }} [options]
 * @param {(
 *   err?: Error,
 *   stats?: Stats
 *   ) => any} callback
 * @returns {void}
 */
function stat(path, options = { bigint: false }, callback) {
  if (typeof options === 'function') {
    callback = options;
    options = kEmptyObject;
  }
  callback = makeStatsCallback(callback);
  path = getValidatedPath(path);

  const req = new FSReqCallback(options.bigint);
  req.oncomplete = callback;
  binding.stat(pathModule.toNamespacedPath(path), options.bigint, req);
}

function statfs(path, options = { bigint: false }, callback) {
  if (typeof options === 'function') {
    callback = options;
    options = kEmptyObject;
  }
  callback = maybeCallback(callback);
  path = getValidatedPath(path);
  const req = new FSReqCallback(options.bigint);
  req.oncomplete = (err, stats) => {
    if (err) {
      return callback(err);
    }

    callback(err, getStatFsFromBinding(stats));
  };
  binding.statfs(pathModule.toNamespacedPath(path), options.bigint, req);
}

function hasNoEntryError(ctx) {
  if (ctx.errno) {
    const uvErr = uvErrmapGet(ctx.errno);
    return uvErr?.[0] === 'ENOENT';
  }

  if (ctx.error) {
    return ctx.error.code === 'ENOENT';
  }

  return false;
}

/**
 * Synchronously retrieves the `fs.Stats` for
 * the file descriptor.
 * @param {number} fd
 * @param {{
 *   bigint?: boolean;
 *   }} [options]
 * @returns {Stats}
 */
function fstatSync(fd, options = { bigint: false }) {
  fd = getValidatedFd(fd);
  const ctx = { fd };
  const stats = binding.fstat(fd, options.bigint, undefined, ctx);
  handleErrorFromBinding(ctx);
  return getStatsFromBinding(stats);
}

/**
 * Synchronously retrieves the `fs.Stats` for
 * the symbolic link referred to by the `path`.
 * @param {string | Buffer | URL} path
 * @param {{
 *   bigint?: boolean;
 *   throwIfNoEntry?: boolean;
 *   }} [options]
 * @returns {Stats}
 */
function lstatSync(path, options = { bigint: false, throwIfNoEntry: true }) {
  path = getValidatedPath(path);
  const ctx = { path };
  const stats = binding.lstat(pathModule.toNamespacedPath(path),
                              options.bigint, undefined, ctx);
  if (options.throwIfNoEntry === false && hasNoEntryError(ctx)) {
    return undefined;
  }
  handleErrorFromBinding(ctx);
  return getStatsFromBinding(stats);
}

/**
 * Synchronously retrieves the `fs.Stats`
 * for the `path`.
 * @param {string | Buffer | URL} path
 * @param {{
 *   bigint?: boolean;
 *   throwIfNoEntry?: boolean;
 *   }} [options]
 * @returns {Stats}
 */
function statSync(path, options = { bigint: false, throwIfNoEntry: true }) {
  path = getValidatedPath(path);
  const ctx = { path };
  const stats = binding.stat(pathModule.toNamespacedPath(path),
                             options.bigint, undefined, ctx);
  if (options.throwIfNoEntry === false && hasNoEntryError(ctx)) {
    return undefined;
  }
  handleErrorFromBinding(ctx);
  return getStatsFromBinding(stats);
}

function statfsSync(path, options = { bigint: false }) {
  path = getValidatedPath(path);
  const ctx = { path };
  const stats = binding.statfs(pathModule.toNamespacedPath(path),
                               options.bigint, undefined, ctx);
  handleErrorFromBinding(ctx);
  return getStatFsFromBinding(stats);
}

/**
 * Reads the contents of a symbolic link
 * referred to by `path`.
 * @param {string | Buffer | URL} path
 * @param {{ encoding?: string; } | string} [options]
 * @param {(
 *   err?: Error,
 *   linkString?: string | Buffer
 *   ) => any} callback
 * @returns {void}
 */
function readlink(path, options, callback) {
  callback = makeCallback(typeof options === 'function' ? options : callback);
  options = getOptions(options);
  path = getValidatedPath(path, 'oldPath');
  const req = new FSReqCallback();
  req.oncomplete = callback;
  binding.readlink(pathModule.toNamespacedPath(path), options.encoding, req);
}

/**
 * Synchronously reads the contents of a symbolic link
 * referred to by `path`.
 * @param {string | Buffer | URL} path
 * @param {{ encoding?: string; } | string} [options]
 * @returns {string | Buffer}
 */
function readlinkSync(path, options) {
  options = getOptions(options);
  path = getValidatedPath(path, 'oldPath');
  const ctx = { path };
  const result = binding.readlink(pathModule.toNamespacedPath(path),
                                  options.encoding, undefined, ctx);
  handleErrorFromBinding(ctx);
  return result;
}

/**
 * Creates the link called `path` pointing to `target`.
 * @param {string | Buffer | URL} target
 * @param {string | Buffer | URL} path
 * @param {string | null} [type_]
 * @param {(err?: Error) => any} callback_
 * @returns {void}
 */
function symlink(target, path, type_, callback_) {
  const type = (typeof type_ === 'string' ? type_ : null);
  const callback = makeCallback(arguments[arguments.length - 1]);

  target = getValidatedPath(target, 'target');
  path = getValidatedPath(path);

  if (isWindows && type === null) {
    let absoluteTarget;
    try {
      // Symlinks targets can be relative to the newly created path.
      // Calculate absolute file name of the symlink target, and check
      // if it is a directory. Ignore resolve error to keep symlink
      // errors consistent between platforms if invalid path is
      // provided.
      absoluteTarget = pathModule.resolve(path, '..', target);
    } catch {
      // Continue regardless of error.
    }
    if (absoluteTarget !== undefined) {
      stat(absoluteTarget, (err, stat) => {
        const resolvedType = !err && stat.isDirectory() ? 'dir' : 'file';
        const resolvedFlags = stringToSymlinkType(resolvedType);
        const destination = preprocessSymlinkDestination(target,
                                                         resolvedType,
                                                         path);

        const req = new FSReqCallback();
        req.oncomplete = callback;
        binding.symlink(destination,
                        pathModule.toNamespacedPath(path), resolvedFlags, req);
      });
      return;
    }
  }

  const destination = preprocessSymlinkDestination(target, type, path);

  const flags = stringToSymlinkType(type);
  const req = new FSReqCallback();
  req.oncomplete = callback;
  binding.symlink(destination, pathModule.toNamespacedPath(path), flags, req);
}

/**
 * Synchronously creates the link called `path`
 * pointing to `target`.
 * @param {string | Buffer | URL} target
 * @param {string | Buffer | URL} path
 * @param {string | null} [type]
 * @returns {void}
 */
function symlinkSync(target, path, type) {
  type = (typeof type === 'string' ? type : null);
  if (isWindows && type === null) {
    const absoluteTarget = pathModule.resolve(`${path}`, '..', `${target}`);
    if (statSync(absoluteTarget, { throwIfNoEntry: false })?.isDirectory()) {
      type = 'dir';
    }
  }
  target = getValidatedPath(target, 'target');
  path = getValidatedPath(path);
  const flags = stringToSymlinkType(type);

  const ctx = { path: target, dest: path };
  binding.symlink(preprocessSymlinkDestination(target, type, path),
                  pathModule.toNamespacedPath(path), flags, undefined, ctx);

  handleErrorFromBinding(ctx);
}

/**
 * Creates a new link from the `existingPath`
 * to the `newPath`.
 * @param {string | Buffer | URL} existingPath
 * @param {string | Buffer | URL} newPath
 * @param {(err?: Error) => any} callback
 * @returns {void}
 */
function link(existingPath, newPath, callback) {
  callback = makeCallback(callback);

  existingPath = getValidatedPath(existingPath, 'existingPath');
  newPath = getValidatedPath(newPath, 'newPath');

  const req = new FSReqCallback();
  req.oncomplete = callback;

  binding.link(pathModule.toNamespacedPath(existingPath),
               pathModule.toNamespacedPath(newPath),
               req);
}

/**
 * Synchronously creates a new link from the `existingPath`
 * to the `newPath`.
 * @param {string | Buffer | URL} existingPath
 * @param {string | Buffer | URL} newPath
 * @returns {void}
 */
function linkSync(existingPath, newPath) {
  existingPath = getValidatedPath(existingPath, 'existingPath');
  newPath = getValidatedPath(newPath, 'newPath');

  const ctx = { path: existingPath, dest: newPath };
  const result = binding.link(pathModule.toNamespacedPath(existingPath),
                              pathModule.toNamespacedPath(newPath),
                              undefined, ctx);
  handleErrorFromBinding(ctx);
  return result;
}

/**
 * Asynchronously removes a file or symbolic link.
 * @param {string | Buffer | URL} path
 * @param {(err?: Error) => any} callback
 * @returns {void}
 */
function unlink(path, callback) {
  callback = makeCallback(callback);
  path = getValidatedPath(path);
  const req = new FSReqCallback();
  req.oncomplete = callback;
  binding.unlink(pathModule.toNamespacedPath(path), req);
}

/**
 * Synchronously removes a file or symbolic link.
 * @param {string | Buffer | URL} path
 * @returns {void}
 */
function unlinkSync(path) {
  path = getValidatedPath(path);
  const ctx = { path };
  binding.unlink(pathModule.toNamespacedPath(path), undefined, ctx);
  handleErrorFromBinding(ctx);
}

/**
 * Sets the permissions on the file.
 * @param {number} fd
 * @param {string | number} mode
 * @param {(err?: Error) => any} callback
 * @returns {void}
 */
function fchmod(fd, mode, callback) {
  fd = getValidatedFd(fd);
  mode = parseFileMode(mode, 'mode');
  callback = makeCallback(callback);

  const req = new FSReqCallback();
  req.oncomplete = callback;
  binding.fchmod(fd, mode, req);
}

/**
 * Synchronously sets the permissions on the file.
 * @param {number} fd
 * @param {string | number} mode
 * @returns {void}
 */
function fchmodSync(fd, mode) {
  fd = getValidatedFd(fd);
  mode = parseFileMode(mode, 'mode');
  const ctx = {};
  binding.fchmod(fd, mode, undefined, ctx);
  handleErrorFromBinding(ctx);
}

/**
 * Changes the permissions on a symbolic link.
 * @param {string | Buffer | URL} path
 * @param {number} mode
 * @param {(err?: Error) => any} callback
 * @returns {void}