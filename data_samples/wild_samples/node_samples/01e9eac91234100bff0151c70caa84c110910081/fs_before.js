function fchmod(fd, mode, callback) {
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
  binding.fchmod(
    fd,
    parseFileMode(mode, 'mode'),
  );
}

/**
 * Changes the permissions on a symbolic link.
 * @param {string | Buffer | URL} path
 * @param {number} mode
 * @param {(err?: Error) => any} callback
 * @returns {void}
 */
function lchmod(path, mode, callback) {
  validateFunction(callback, 'cb');
  mode = parseFileMode(mode, 'mode');
  fs.open(path, O_WRONLY | O_SYMLINK, (err, fd) => {
    if (err) {
      callback(err);
      return;
    }
    // Prefer to return the chmod error, if one occurs,
    // but still try to close, and report closing errors if they occur.
    fs.fchmod(fd, mode, (err) => {
      fs.close(fd, (err2) => {
        callback(aggregateTwoErrors(err2, err));
      });
    });
  });
}
function fchmod(fd, mode, callback) {
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
  binding.fchmod(
    fd,
    parseFileMode(mode, 'mode'),
  );
}
function fchown(fd, uid, gid, callback) {
  validateInteger(uid, 'uid', -1, kMaxUserId);
  validateInteger(gid, 'gid', -1, kMaxUserId);
  callback = makeCallback(callback);

  const req = new FSReqCallback();
  req.oncomplete = callback;
  binding.fchown(fd, uid, gid, req);
}

/**
 * Synchronously sets the owner of the file.
 * @param {number} fd
 * @param {number} uid
 * @param {number} gid
 * @returns {void}
 */
function fchownSync(fd, uid, gid) {
  validateInteger(uid, 'uid', -1, kMaxUserId);
  validateInteger(gid, 'gid', -1, kMaxUserId);

  binding.fchown(fd, uid, gid);
}

/**
 * Asynchronously changes the owner and group
 * of a file.
 * @param {string | Buffer | URL} path
 * @param {number} uid
 * @param {number} gid
 * @param {(err?: Error) => any} callback
 * @returns {void}
 */
function chown(path, uid, gid, callback) {
  callback = makeCallback(callback);
  path = getValidatedPath(path);
  validateInteger(uid, 'uid', -1, kMaxUserId);
  validateInteger(gid, 'gid', -1, kMaxUserId);

  const req = new FSReqCallback();
  req.oncomplete = callback;
  binding.chown(path, uid, gid, req);
}

/**
 * Synchronously changes the owner and group
 * of a file.
 * @param {string | Buffer | URL} path
 * @param {number} uid
 * @param {number} gid
 * @returns {void}
 */
function chownSync(path, uid, gid) {
  path = getValidatedPath(path);
  validateInteger(uid, 'uid', -1, kMaxUserId);
  validateInteger(gid, 'gid', -1, kMaxUserId);
  binding.chown(path, uid, gid);
}

/**
 * Changes the file system timestamps of the object
 * referenced by `path`.
 * @param {string | Buffer | URL} path
 * @param {number | string | Date} atime
 * @param {number | string | Date} mtime
 * @param {(err?: Error) => any} callback
 * @returns {void}
 */
function utimes(path, atime, mtime, callback) {
  callback = makeCallback(callback);
  path = getValidatedPath(path);

  const req = new FSReqCallback();
  req.oncomplete = callback;
  binding.utimes(
    path,
    toUnixTimestamp(atime),
    toUnixTimestamp(mtime),
    req,
  );
}

/**
 * Synchronously changes the file system timestamps
 * of the object referenced by `path`.
 * @param {string | Buffer | URL} path
 * @param {number | string | Date} atime
 * @param {number | string | Date} mtime
 * @returns {void}
 */
function utimesSync(path, atime, mtime) {
  binding.utimes(
    getValidatedPath(path),
    toUnixTimestamp(atime),
    toUnixTimestamp(mtime),
  );
}

/**
 * Changes the file system timestamps of the object
 * referenced by the supplied `fd` (file descriptor).
 * @param {number} fd
 * @param {number | string | Date} atime
 * @param {number | string | Date} mtime
 * @param {(err?: Error) => any} callback
 * @returns {void}
 */
function futimes(fd, atime, mtime, callback) {
  atime = toUnixTimestamp(atime, 'atime');
  mtime = toUnixTimestamp(mtime, 'mtime');
  callback = makeCallback(callback);

  const req = new FSReqCallback();
  req.oncomplete = callback;
  binding.futimes(fd, atime, mtime, req);
}

/**
 * Synchronously changes the file system timestamps
 * of the object referenced by the
 * supplied `fd` (file descriptor).
 * @param {number} fd
 * @param {number | string | Date} atime
 * @param {number | string | Date} mtime
 * @returns {void}
 */
function futimesSync(fd, atime, mtime) {
  binding.futimes(
    fd,
    toUnixTimestamp(atime, 'atime'),
    toUnixTimestamp(mtime, 'mtime'),
  );
}

/**
 * Changes the access and modification times of
 * a file in the same way as `fs.utimes()`.
 * @param {string | Buffer | URL} path
 * @param {number | string | Date} atime
 * @param {number | string | Date} mtime
 * @param {(err?: Error) => any} callback
 * @returns {void}
 */
function lutimes(path, atime, mtime, callback) {
  callback = makeCallback(callback);
  path = getValidatedPath(path);

  const req = new FSReqCallback();
  req.oncomplete = callback;
  binding.lutimes(
    path,
    toUnixTimestamp(atime),
    toUnixTimestamp(mtime),
    req,
  );
}

/**
 * Synchronously changes the access and modification
 * times of a file in the same way as `fs.utimesSync()`.
 * @param {string | Buffer | URL} path
 * @param {number | string | Date} atime
 * @param {number | string | Date} mtime
 * @returns {void}
 */
function lutimesSync(path, atime, mtime) {
  binding.lutimes(
    getValidatedPath(path),
    toUnixTimestamp(atime),
    toUnixTimestamp(mtime),
  );
}

function writeAll(fd, isUserFd, buffer, offset, length, signal, flush, callback) {
  if (signal?.aborted) {
    const abortError = new AbortError(undefined, { cause: signal?.reason });
    if (isUserFd) {
      callback(abortError);
    } else {
      fs.close(fd, (err) => {
        callback(aggregateTwoErrors(err, abortError));
      });
    }
    return;
  }
  // write(fd, buffer, offset, length, position, callback)
  fs.write(fd, buffer, offset, length, null, (writeErr, written) => {
    if (writeErr) {
      if (isUserFd) {
        callback(writeErr);
      } else {
        fs.close(fd, (err) => {
          callback(aggregateTwoErrors(err, writeErr));
        });
      }
    } else if (written === length) {
      if (!flush) {
        if (isUserFd) {
          callback(null);
        } else {
          fs.close(fd, callback);
        }
function fchownSync(fd, uid, gid) {
  validateInteger(uid, 'uid', -1, kMaxUserId);
  validateInteger(gid, 'gid', -1, kMaxUserId);

  binding.fchown(fd, uid, gid);
}

/**
 * Asynchronously changes the owner and group
 * of a file.
 * @param {string | Buffer | URL} path
 * @param {number} uid
 * @param {number} gid
 * @param {(err?: Error) => any} callback
 * @returns {void}
 */
function chown(path, uid, gid, callback) {
  callback = makeCallback(callback);
  path = getValidatedPath(path);
  validateInteger(uid, 'uid', -1, kMaxUserId);
  validateInteger(gid, 'gid', -1, kMaxUserId);

  const req = new FSReqCallback();
  req.oncomplete = callback;
  binding.chown(path, uid, gid, req);
}

/**
 * Synchronously changes the owner and group
 * of a file.
 * @param {string | Buffer | URL} path
 * @param {number} uid
 * @param {number} gid
 * @returns {void}
 */
function chownSync(path, uid, gid) {
  path = getValidatedPath(path);
  validateInteger(uid, 'uid', -1, kMaxUserId);
  validateInteger(gid, 'gid', -1, kMaxUserId);
  binding.chown(path, uid, gid);
}

/**
 * Changes the file system timestamps of the object
 * referenced by `path`.
 * @param {string | Buffer | URL} path
 * @param {number | string | Date} atime
 * @param {number | string | Date} mtime
 * @param {(err?: Error) => any} callback
 * @returns {void}
 */
function utimes(path, atime, mtime, callback) {
  callback = makeCallback(callback);
  path = getValidatedPath(path);

  const req = new FSReqCallback();
  req.oncomplete = callback;
  binding.utimes(
    path,
    toUnixTimestamp(atime),
    toUnixTimestamp(mtime),
    req,
  );
}

/**
 * Synchronously changes the file system timestamps
 * of the object referenced by `path`.
 * @param {string | Buffer | URL} path
 * @param {number | string | Date} atime
 * @param {number | string | Date} mtime
 * @returns {void}
 */
function utimesSync(path, atime, mtime) {
  binding.utimes(
    getValidatedPath(path),
    toUnixTimestamp(atime),
    toUnixTimestamp(mtime),
  );
}

/**
 * Changes the file system timestamps of the object
 * referenced by the supplied `fd` (file descriptor).
 * @param {number} fd
 * @param {number | string | Date} atime
 * @param {number | string | Date} mtime
 * @param {(err?: Error) => any} callback
 * @returns {void}
 */
function futimes(fd, atime, mtime, callback) {
  atime = toUnixTimestamp(atime, 'atime');
  mtime = toUnixTimestamp(mtime, 'mtime');
  callback = makeCallback(callback);

  const req = new FSReqCallback();
  req.oncomplete = callback;
  binding.futimes(fd, atime, mtime, req);
}

/**
 * Synchronously changes the file system timestamps
 * of the object referenced by the
 * supplied `fd` (file descriptor).
 * @param {number} fd
 * @param {number | string | Date} atime
 * @param {number | string | Date} mtime
 * @returns {void}
 */
function futimesSync(fd, atime, mtime) {
  binding.futimes(
    fd,
    toUnixTimestamp(atime, 'atime'),
    toUnixTimestamp(mtime, 'mtime'),
  );
}

/**
 * Changes the access and modification times of
 * a file in the same way as `fs.utimes()`.
 * @param {string | Buffer | URL} path
 * @param {number | string | Date} atime
 * @param {number | string | Date} mtime
 * @param {(err?: Error) => any} callback
 * @returns {void}
 */
function lutimes(path, atime, mtime, callback) {
  callback = makeCallback(callback);
  path = getValidatedPath(path);

  const req = new FSReqCallback();
  req.oncomplete = callback;
  binding.lutimes(
    path,
    toUnixTimestamp(atime),
    toUnixTimestamp(mtime),
    req,
  );
}

/**
 * Synchronously changes the access and modification
 * times of a file in the same way as `fs.utimesSync()`.
 * @param {string | Buffer | URL} path
 * @param {number | string | Date} atime
 * @param {number | string | Date} mtime
 * @returns {void}
 */
function lutimesSync(path, atime, mtime) {
  binding.lutimes(
    getValidatedPath(path),
    toUnixTimestamp(atime),
    toUnixTimestamp(mtime),
  );
}

function writeAll(fd, isUserFd, buffer, offset, length, signal, flush, callback) {
  if (signal?.aborted) {
    const abortError = new AbortError(undefined, { cause: signal?.reason });
    if (isUserFd) {
      callback(abortError);
    } else {
      fs.close(fd, (err) => {
        callback(aggregateTwoErrors(err, abortError));
      });
    }
    return;
  }
  // write(fd, buffer, offset, length, position, callback)
  fs.write(fd, buffer, offset, length, null, (writeErr, written) => {
    if (writeErr) {
      if (isUserFd) {
        callback(writeErr);
      } else {
        fs.close(fd, (err) => {
          callback(aggregateTwoErrors(err, writeErr));
        });
      }
    } else if (written === length) {
      if (!flush) {
        if (isUserFd) {
          callback(null);
        } else {
          fs.close(fd, callback);
        }
      } else {
        fs.fsync(fd, (syncErr) => {
          if (syncErr) {
            if (isUserFd) {
              callback(syncErr);
            } else {
              fs.close(fd, (err) => {
                callback(aggregateTwoErrors(err, syncErr));
              });
            }
          } else if (isUserFd) {
            callback(null);
          } else {
            fs.close(fd, callback);
          }
        });
      }
    } else {
      offset += written;
      length -= written;
      writeAll(fd, isUserFd, buffer, offset, length, signal, flush, callback);
    }
  });
}

/**
 * Asynchronously writes data to the file.
 * @param {string | Buffer | URL | number} path
 * @param {string | Buffer | TypedArray | DataView} data
 * @param {{
 *   encoding?: string | null;
 *   mode?: number;
 *   flag?: string;
 *   signal?: AbortSignal;
 *   flush?: boolean;
 *   } | string} [options]
 * @param {(err?: Error) => any} callback
 * @returns {void}
 */
function writeFile(path, data, options, callback) {
  callback ||= options;
  validateFunction(callback, 'cb');
  options = getOptions(options, {
    encoding: 'utf8',
    mode: 0o666,
    flag: 'w',
    flush: false,
  });
  const flag = options.flag || 'w';
  const flush = options.flush ?? false;

  validateBoolean(flush, 'options.flush');

  if (!isArrayBufferView(data)) {
    validateStringAfterArrayBufferView(data, 'data');
    data = Buffer.from(data, options.encoding || 'utf8');
  }

  if (isFd(path)) {
    const isUserFd = true;
    const signal = options.signal;
    writeAll(path, isUserFd, data, 0, data.byteLength, signal, flush, callback);
    return;
  }

  if (checkAborted(options.signal, callback))
    return;

  fs.open(path, flag, options.mode, (openErr, fd) => {
    if (openErr) {
      callback(openErr);
    } else {
      const isUserFd = false;
      const signal = options.signal;
      writeAll(fd, isUserFd, data, 0, data.byteLength, signal, flush, callback);
    }
  });
}

/**
 * Synchronously writes data to the file.
 * @param {string | Buffer | URL | number} path
 * @param {string | Buffer | TypedArray | DataView} data
 * @param {{
 *   encoding?: string | null;
 *   mode?: number;
 *   flag?: string;
 *   flush?: boolean;
 *   } | string} [options]
 * @returns {void}
 */
function writeFileSync(path, data, options) {
  options = getOptions(options, {
    encoding: 'utf8',
    mode: 0o666,
    flag: 'w',
    flush: false,
  });

  const flush = options.flush ?? false;

  validateBoolean(flush, 'options.flush');

  const flag = options.flag || 'w';

  // C++ fast path for string data and UTF8 encoding
  if (typeof data === 'string' && (options.encoding === 'utf8' || options.encoding === 'utf-8')) {
    if (!isInt32(path)) {
      path = getValidatedPath(path);
    }

    return binding.writeFileUtf8(
      path,
      data,
      stringToFlags(flag),
      parseFileMode(options.mode, 'mode', 0o666),
    );
  }

  if (!isArrayBufferView(data)) {
    validateStringAfterArrayBufferView(data, 'data');
    data = Buffer.from(data, options.encoding || 'utf8');
  }

  const isUserFd = isFd(path); // File descriptor ownership
  const fd = isUserFd ? path : fs.openSync(path, flag, options.mode);

  let offset = 0;
  let length = data.byteLength;
  try {
    while (length > 0) {
      const written = fs.writeSync(fd, data, offset, length);
      offset += written;
      length -= written;
    }

    if (flush) {
      fs.fsyncSync(fd);
    }
  } finally {
    if (!isUserFd) fs.closeSync(fd);
  }
}

/**
 * Asynchronously appends data to a file.
 * @param {string | Buffer | URL | number} path
 * @param {string | Buffer} data
 * @param {{
 *   encoding?: string | null;
 *   mode?: number;
 *   flag?: string;
 *   flush?: boolean;
 *   } | string} [options]
 * @param {(err?: Error) => any} callback
 * @returns {void}
 */
function appendFile(path, data, options, callback) {
  callback ||= options;
  validateFunction(callback, 'cb');
  options = getOptions(options, { encoding: 'utf8', mode: 0o666, flag: 'a' });

  // Don't make changes directly on options object
  options = copyObject(options);

  // Force append behavior when using a supplied file descriptor
  if (!options.flag || isFd(path))
    options.flag = 'a';

  fs.writeFile(path, data, options, callback);
}

/**
 * Synchronously appends data to a file.
 * @param {string | Buffer | URL | number} path
 * @param {string | Buffer} data
 * @param {{
 *   encoding?: string | null;
 *   mode?: number;
 *   flag?: string;
 *   } | string} [options]
 * @returns {void}
 */
function appendFileSync(path, data, options) {
  options = getOptions(options, { encoding: 'utf8', mode: 0o666, flag: 'a' });

  // Don't make changes directly on options object
  options = copyObject(options);

  // Force append behavior when using a supplied file descriptor
  if (!options.flag || isFd(path))
    options.flag = 'a';

  fs.writeFileSync(path, data, options);
}

/**
 * Watches for the changes on `filename`.
 * @param {string | Buffer | URL} filename
 * @param {string | {
 *   persistent?: boolean;
 *   recursive?: boolean;
 *   encoding?: string;
 *   signal?: AbortSignal;
 *   }} [options]
 * @param {(
 *   eventType?: string,
 *   filename?: string | Buffer
 *   ) => any} [listener]
 * @returns {watchers.FSWatcher}
 */
function watch(filename, options, listener) {
  if (typeof options === 'function') {
    listener = options;
  }
  options = getOptions(options);

  // Don't make changes directly on options object
  options = copyObject(options);

  if (options.persistent === undefined) options.persistent = true;
  if (options.recursive === undefined) options.recursive = false;

  let watcher;
  const watchers = require('internal/fs/watchers');
  const path = getValidatedPath(filename);
  // TODO(anonrig): Remove non-native watcher when/if libuv supports recursive.
  // As of November 2022, libuv does not support recursive file watch on all platforms,
  // e.g. Linux due to the limitations of inotify.
  if (options.recursive && !isOSX && !isWindows) {
    const nonNativeWatcher = require('internal/fs/recursive_watch');
    watcher = new nonNativeWatcher.FSWatcher(options);
    watcher[watchers.kFSWatchStart](path);
  } else {
    watcher = new watchers.FSWatcher();
    watcher[watchers.kFSWatchStart](path,
                                    options.persistent,
                                    options.recursive,
                                    options.encoding);
  }

  if (listener) {
    watcher.addListener('change', listener);
  }
  if (options.signal) {
    if (options.signal.aborted) {
      process.nextTick(() => watcher.close());
    } else {
      const listener = () => watcher.close();
      kResistStopPropagation ??= require('internal/event_target').kResistStopPropagation;
      options.signal.addEventListener('abort', listener, { __proto__: null, [kResistStopPropagation]: true });
      watcher.once('close', () => {
        options.signal.removeEventListener('abort', listener);
      });
    }
  }

  return watcher;
}


const statWatchers = new SafeMap();

/**
 * Watches for changes on `filename`.
 * @param {string | Buffer | URL} filename
 * @param {{
 *   bigint?: boolean;
 *   persistent?: boolean;
 *   interval?: number;
 *   }} [options]
 * @param {(
 *   current?: Stats,
 *   previous?: Stats
 *   ) => any} listener
 * @returns {watchers.StatWatcher}
 */
function watchFile(filename, options, listener) {
  filename = getValidatedPath(filename);
  filename = pathModule.resolve(filename);
  let stat;

  if (options === null || typeof options !== 'object') {
    listener = options;
    options = null;
  }

  options = {
    // Poll interval in milliseconds. 5007 is what libev used to use. It's
    // a little on the slow side but let's stick with it for now to keep
    // behavioral changes to a minimum.
    interval: 5007,
    persistent: true,
    ...options,
  };

  validateFunction(listener, 'listener');

  stat = statWatchers.get(filename);
  const watchers = require('internal/fs/watchers');
  if (stat === undefined) {
    stat = new watchers.StatWatcher(options.bigint);
    stat[watchers.kFSStatWatcherStart](filename,
                                       options.persistent, options.interval);
    statWatchers.set(filename, stat);
  } else {
    stat[watchers.kFSStatWatcherAddOrCleanRef]('add');
  }

  stat.addListener('change', listener);
  return stat;
}

/**
 * Stops watching for changes on `filename`.
 * @param {string | Buffer | URL} filename
 * @param {() => any} [listener]
 * @returns {void}
 */
function unwatchFile(filename, listener) {
  filename = getValidatedPath(filename);
  filename = pathModule.resolve(filename);
  const stat = statWatchers.get(filename);

  if (stat === undefined) return;
  const watchers = require('internal/fs/watchers');
  if (typeof listener === 'function') {
    const beforeListenerCount = stat.listenerCount('change');
    stat.removeListener('change', listener);
    if (stat.listenerCount('change') < beforeListenerCount)
      stat[watchers.kFSStatWatcherAddOrCleanRef]('clean');
  } else {
    stat.removeAllListeners('change');
    stat[watchers.kFSStatWatcherAddOrCleanRef]('cleanAll');
  }

  if (stat.listenerCount('change') === 0) {
    stat.stop();
    statWatchers.delete(filename);
  }
}


let splitRoot;
if (isWindows) {
  // Regex to find the device root on Windows (e.g. 'c:\\'), including trailing
  // slash.
  const splitRootRe = /^(?:[a-zA-Z]:|[\\/]{2}[^\\/]+[\\/][^\\/]+)?[\\/]*/;
  splitRoot = function splitRoot(str) {
    return SideEffectFreeRegExpPrototypeExec(splitRootRe, str)[0];
  };
} else {
  splitRoot = function splitRoot(str) {
    for (let i = 0; i < str.length; ++i) {
      if (StringPrototypeCharCodeAt(str, i) !== CHAR_FORWARD_SLASH)
        return StringPrototypeSlice(str, 0, i);
    }
    return str;
  };
}

function encodeRealpathResult(result, options) {
  if (!options || !options.encoding || options.encoding === 'utf8')
    return result;
  const asBuffer = Buffer.from(result);
  if (options.encoding === 'buffer') {
    return asBuffer;
  }
  return asBuffer.toString(options.encoding);
}

// Finds the next portion of a (partial) path, up to the next path delimiter
let nextPart;
if (isWindows) {
  nextPart = function nextPart(p, i) {
    for (; i < p.length; ++i) {
      const ch = StringPrototypeCharCodeAt(p, i);

      // Check for a separator character
      if (ch === CHAR_BACKWARD_SLASH || ch === CHAR_FORWARD_SLASH)
        return i;
    }
    return -1;
  };
} else {
  nextPart = function nextPart(p, i) {
    return StringPrototypeIndexOf(p, '/', i);
  };
}

/**
 * Returns the resolved pathname.
 * @param {string | Buffer | URL} p
 * @param {string | { encoding?: string | null; }} [options]
 * @returns {string | Buffer}
 */
function realpathSync(p, options) {
  options = getOptions(options);
  p = toPathIfFileURL(p);
  if (typeof p !== 'string') {
    p += '';
  }
  validatePath(p);
  p = pathModule.resolve(p);

  const cache = options[realpathCacheKey];
  const maybeCachedResult = cache?.get(p);
  if (maybeCachedResult) {
    return maybeCachedResult;
  }

  const seenLinks = new SafeMap();
  const knownHard = new SafeSet();
  const original = p;

  // Current character position in p
  let pos;
  // The partial path so far, including a trailing slash if any
  let current;
  // The partial path without a trailing slash (except when pointing at a root)
  let base;
  // The partial path scanned in the previous round, with slash
  let previous;

  // Skip over roots
  current = base = splitRoot(p);
  pos = current.length;

  // On windows, check that the root exists. On unix there is no need.
  if (isWindows) {
    const out = binding.lstat(base, false, undefined, true /* throwIfNoEntry */);
    if (out === undefined) {
      return;
    }
    knownHard.add(base);
  }

  // Walk down the path, swapping out linked path parts for their real
  // values
  // NB: p.length changes.
  while (pos < p.length) {
    // find the next part
    const result = nextPart(p, pos);
    previous = current;
    if (result === -1) {
      const last = StringPrototypeSlice(p, pos);
      current += last;
      base = previous + last;
      pos = p.length;
    } else {
      current += StringPrototypeSlice(p, pos, result + 1);
      base = previous + StringPrototypeSlice(p, pos, result);
      pos = result + 1;
    }

    // Continue if not a symlink, break if a pipe/socket
    if (knownHard.has(base) || cache?.get(base) === base) {
      if (isFileType(statValues, S_IFIFO) ||
          isFileType(statValues, S_IFSOCK)) {
        break;
      }
      continue;
    }

    let resolvedLink;
    const maybeCachedResolved = cache?.get(base);
    if (maybeCachedResolved) {
      resolvedLink = maybeCachedResolved;
    } else {
      // Use stats array directly to avoid creating an fs.Stats instance just
      // for our internal use.

      const stats = binding.lstat(base, true, undefined, true /* throwIfNoEntry */);
      if (stats === undefined) {
        return;
      }

      if (!isFileType(stats, S_IFLNK)) {
        knownHard.add(base);
        cache?.set(base, base);
        continue;
      }

      // Read the link if it wasn't read before
      // dev/ino always return 0 on windows, so skip the check.
      let linkTarget = null;
      let id;
      if (!isWindows) {
        const dev = BigIntPrototypeToString(stats[0], 32);
        const ino = BigIntPrototypeToString(stats[7], 32);
        id = `${dev}:${ino}`;
        if (seenLinks.has(id)) {
          linkTarget = seenLinks.get(id);
        }
      }
      if (linkTarget === null) {
        binding.stat(base, false, undefined, true);
        linkTarget = binding.readlink(base, undefined);
      }
      resolvedLink = pathModule.resolve(previous, linkTarget);

      cache?.set(base, resolvedLink);
      if (!isWindows) seenLinks.set(id, linkTarget);
    }

    // Resolve the link, then start over
    p = pathModule.resolve(resolvedLink, StringPrototypeSlice(p, pos));

    // Skip over roots
    current = base = splitRoot(p);
    pos = current.length;

    // On windows, check that the root exists. On unix there is no need.
    if (isWindows && !knownHard.has(base)) {
      const out = binding.lstat(base, false, undefined, true /* throwIfNoEntry */);
      if (out === undefined) {
        return;
      }
      knownHard.add(base);
    }
  }

  cache?.set(original, p);
  return encodeRealpathResult(p, options);
}

/**
 * Returns the resolved pathname.
 * @param {string | Buffer | URL} path
 * @param {string | { encoding?: string; }} [options]
 * @returns {string | Buffer}
 */
realpathSync.native = (path, options) => {
  options = getOptions(options);
  return binding.realpath(
    getValidatedPath(path),
    options.encoding,
  );
};

/**
 * Asynchronously computes the canonical pathname by
 * resolving `.`, `..` and symbolic links.
 * @param {string | Buffer | URL} p
 * @param {string | { encoding?: string; }} [options]
 * @param {(
 *   err?: Error,
 *   resolvedPath?: string | Buffer
 *   ) => any} callback
 * @returns {void}
 */
function realpath(p, options, callback) {
  if (typeof options === 'function') {
    callback = options;
  } else {
    validateFunction(callback, 'cb');
  }
  options = getOptions(options);
  p = toPathIfFileURL(p);

  if (typeof p !== 'string') {
    p += '';
  }
  validatePath(p);
  p = pathModule.resolve(p);

  const seenLinks = new SafeMap();
  const knownHard = new SafeSet();

  // Current character position in p
  let pos;
  // The partial path so far, including a trailing slash if any
  let current;
  // The partial path without a trailing slash (except when pointing at a root)
  let base;
  // The partial path scanned in the previous round, with slash
  let previous;

  current = base = splitRoot(p);
  pos = current.length;

  // On windows, check that the root exists. On unix there is no need.
  if (isWindows && !knownHard.has(base)) {
    fs.lstat(base, (err) => {
      if (err) return callback(err);
      knownHard.add(base);
      LOOP();
    });
  } else {
    process.nextTick(LOOP);
  }

  // Walk down the path, swapping out linked path parts for their real
  // values
  function LOOP() {
    // Stop if scanned past end of path
    if (pos >= p.length) {
      return callback(null, encodeRealpathResult(p, options));
    }

    // find the next part
    const result = nextPart(p, pos);
    previous = current;
    if (result === -1) {
      const last = StringPrototypeSlice(p, pos);
      current += last;
      base = previous + last;
      pos = p.length;
    } else {
      current += StringPrototypeSlice(p, pos, result + 1);
      base = previous + StringPrototypeSlice(p, pos, result);
      pos = result + 1;
    }

    // Continue if not a symlink, break if a pipe/socket
    if (knownHard.has(base)) {
      if (isFileType(statValues, S_IFIFO) ||
          isFileType(statValues, S_IFSOCK)) {
        return callback(null, encodeRealpathResult(p, options));
      }
      return process.nextTick(LOOP);
    }

    return fs.lstat(base, { bigint: true }, gotStat);
  }

  function gotStat(err, stats) {
    if (err) return callback(err);

    // If not a symlink, skip to the next path part
    if (!stats.isSymbolicLink()) {
      knownHard.add(base);
      return process.nextTick(LOOP);
    }

    // Stat & read the link if not read before.
    // Call `gotTarget()` as soon as the link target is known.
    // `dev`/`ino` always return 0 on windows, so skip the check.
    let id;
    if (!isWindows) {
      const dev = BigIntPrototypeToString(stats.dev, 32);
      const ino = BigIntPrototypeToString(stats.ino, 32);
      id = `${dev}:${ino}`;
      if (seenLinks.has(id)) {
        return gotTarget(null, seenLinks.get(id));
      }
    }
    fs.stat(base, (err) => {
      if (err) return callback(err);

      fs.readlink(base, (err, target) => {
        if (!isWindows) seenLinks.set(id, target);
        gotTarget(err, target);
      });
    });
  }

  function gotTarget(err, target) {
    if (err) return callback(err);

    gotResolvedLink(pathModule.resolve(previous, target));
  }

  function gotResolvedLink(resolvedLink) {
    // Resolve the link, then start over
    p = pathModule.resolve(resolvedLink, StringPrototypeSlice(p, pos));
    current = base = splitRoot(p);
    pos = current.length;

    // On windows, check that the root exists. On unix there is no need.
    if (isWindows && !knownHard.has(base)) {
      fs.lstat(base, (err) => {
        if (err) return callback(err);
        knownHard.add(base);
        LOOP();
      });
    } else {
      process.nextTick(LOOP);
    }
  }
}

/**
 * Asynchronously computes the canonical pathname by
 * resolving `.`, `..` and symbolic links.
 * @param {string | Buffer | URL} path
 * @param {string | { encoding?: string; }} [options]
 * @param {(
 *   err?: Error,
 *   resolvedPath?: string | Buffer
 *   ) => any} callback
 * @returns {void}
 */
realpath.native = (path, options, callback) => {
  callback = makeCallback(callback || options);
  options = getOptions(options);
  path = getValidatedPath(path);
  const req = new FSReqCallback();
  req.oncomplete = callback;
  binding.realpath(path, options.encoding, req);
};

/**
 * Creates a unique temporary directory.
 * @param {string | Buffer | URL} prefix
 * @param {string | { encoding?: string; }} [options]
 * @param {(
 *   err?: Error,
 *   directory?: string
 *   ) => any} callback
 * @returns {void}
 */
function mkdtemp(prefix, options, callback) {
  callback = makeCallback(typeof options === 'function' ? options : callback);
  options = getOptions(options);

  prefix = getValidatedPath(prefix, 'prefix');
  warnOnNonPortableTemplate(prefix);

  const req = new FSReqCallback();
  req.oncomplete = callback;
  binding.mkdtemp(prefix, options.encoding, req);
}

/**
 * Synchronously creates a unique temporary directory.
 * @param {string | Buffer | URL} prefix
 * @param {string | { encoding?: string; }} [options]
 * @returns {string}
 */
function mkdtempSync(prefix, options) {
  options = getOptions(options);

  prefix = getValidatedPath(prefix, 'prefix');
  warnOnNonPortableTemplate(prefix);
  return binding.mkdtemp(prefix, options.encoding);
}

/**
 * Asynchronously copies `src` to `dest`. By
 * default, `dest` is overwritten if it already exists.
 * @param {string | Buffer | URL} src
 * @param {string | Buffer | URL} dest
 * @param {number} [mode]
 * @param {(err?: Error) => any} callback
 * @returns {void}
 */
function copyFile(src, dest, mode, callback) {
  if (typeof mode === 'function') {
    callback = mode;
    mode = 0;
  }

  src = getValidatedPath(src, 'src');
  dest = getValidatedPath(dest, 'dest');
  callback = makeCallback(callback);

  const req = new FSReqCallback();
  req.oncomplete = callback;
  binding.copyFile(src, dest, mode, req);
}

/**
 * Synchronously copies `src` to `dest`. By
 * default, `dest` is overwritten if it already exists.
 * @param {string | Buffer | URL} src
 * @param {string | Buffer | URL} dest
 * @param {number} [mode]
 * @returns {void}
 */
function copyFileSync(src, dest, mode) {
  binding.copyFile(
    getValidatedPath(src, 'src'),
    getValidatedPath(dest, 'dest'),
    mode,
  );
}

/**
 * Asynchronously copies `src` to `dest`. `src` can be a file, directory, or
 * symlink. The contents of directories will be copied recursively.
 * @param {string | URL} src
 * @param {string | URL} dest
 * @param {object} [options]
 * @param {(err?: Error) => any} callback
 * @returns {void}
 */
function cp(src, dest, options, callback) {
  if (typeof options === 'function') {
    callback = options;
    options = undefined;
  }
  callback = makeCallback(callback);
  options = validateCpOptions(options);
  src = getValidatedPath(src, 'src');
  dest = getValidatedPath(dest, 'dest');
  lazyLoadCp();
  cpFn(src, dest, options, callback);
}

/**
 * Synchronously copies `src` to `dest`. `src` can be a file, directory, or
 * symlink. The contents of directories will be copied recursively.
 * @param {string | URL} src
 * @param {string | URL} dest
 * @param {object} [options]
 * @returns {void}
 */
function cpSync(src, dest, options) {
  options = validateCpOptions(options);
  src = getValidatedPath(src, 'src');
  dest = getValidatedPath(dest, 'dest');
  lazyLoadCp();
  cpSyncFn(src, dest, options);
}

function lazyLoadStreams() {
  if (!ReadStream) {
    ({ ReadStream, WriteStream } = require('internal/fs/streams'));
    FileReadStream = ReadStream;
    FileWriteStream = WriteStream;
  }
}

/**
 * Creates a readable stream with a default `highWaterMark`
 * of 64 KiB.
 * @param {string | Buffer | URL} path
 * @param {string | {
 *   flags?: string;
 *   encoding?: string;
 *   fd?: number | FileHandle;
 *   mode?: number;
 *   autoClose?: boolean;
 *   emitClose?: boolean;
 *   start: number;
 *   end?: number;
 *   highWaterMark?: number;
 *   fs?: object | null;
 *   signal?: AbortSignal | null;
 *   }} [options]
 * @returns {ReadStream}
 */
function createReadStream(path, options) {
  lazyLoadStreams();
  return new ReadStream(path, options);
}

/**
 * Creates a write stream.
 * @param {string | Buffer | URL} path
 * @param {string | {
 *   flags?: string;
 *   encoding?: string;
 *   fd?: number | FileHandle;
 *   mode?: number;
 *   autoClose?: boolean;
 *   emitClose?: boolean;
 *   start: number;
 *   fs?: object | null;
 *   signal?: AbortSignal | null;
 *   highWaterMark?: number;
 *   flush?: boolean;
 *   }} [options]
 * @returns {WriteStream}
 */
function createWriteStream(path, options) {
  lazyLoadStreams();
  return new WriteStream(path, options);
}

const lazyGlob = getLazy(() => require('internal/fs/glob').Glob);

function glob(pattern, options, callback) {
  emitExperimentalWarning('glob');
  if (typeof options === 'function') {
    callback = options;
    options = undefined;
  }
  callback = makeCallback(callback);

  const Glob = lazyGlob();
  // TODO: Use iterator helpers when available
  (async () => {
    try {
      const res = [];
      for await (const entry of new Glob(pattern, options).glob()) {
        ArrayPrototypePush(res, entry);
      }
      callback(null, res);
    } catch (err) {
      callback(err);
    }
  })();
}

function globSync(pattern, options) {
  emitExperimentalWarning('globSync');
  const Glob = lazyGlob();
  return new Glob(pattern, options).globSync();
}


module.exports = fs = {
  appendFile,
  appendFileSync,
  access,
  accessSync,
  chown,
  chownSync,
  chmod,
  chmodSync,
  close,
  closeSync,
  copyFile,
  copyFileSync,
  cp,
  cpSync,
  createReadStream,
  createWriteStream,
  exists,
  existsSync,
  fchown,
  fchownSync,
  fchmod,
  fchmodSync,
  fdatasync,
  fdatasyncSync,
  fstat,
  fstatSync,
  fsync,
  fsyncSync,
  ftruncate,
  ftruncateSync,
  futimes,
  futimesSync,
  glob,
  globSync,
  lchown,
  lchownSync,
  lchmod: constants.O_SYMLINK !== undefined ? lchmod : undefined,
  lchmodSync: constants.O_SYMLINK !== undefined ? lchmodSync : undefined,
  link,
  linkSync,
  lstat,
  lstatSync,
  lutimes,
  lutimesSync,
  mkdir,
  mkdirSync,
  mkdtemp,
  mkdtempSync,
  open,
  openSync,
  openAsBlob,
  readdir,
  readdirSync,
  read,
  readSync,
  readv,
  readvSync,
  readFile,
  readFileSync,
  readlink,
  readlinkSync,
  realpath,
  realpathSync,
  rename,
  renameSync,
  rm,
  rmSync,
  rmdir,
  rmdirSync,
  stat,
  statfs,
  statSync,
  statfsSync,
  symlink,
  symlinkSync,
  truncate,
  truncateSync,
  unwatchFile,
  unlink,
  unlinkSync,
  utimes,
  utimesSync,
  watch,
  watchFile,
  writeFile,
  writeFileSync,
  write,
  writeSync,
  writev,
  writevSync,
  Dirent,
  Stats,

  get ReadStream() {
    lazyLoadStreams();
    return ReadStream;
  },

  set ReadStream(val) {
    ReadStream = val;
  },

  get WriteStream() {
    lazyLoadStreams();
    return WriteStream;
  },

  set WriteStream(val) {
    WriteStream = val;
  },

  // Legacy names... these have to be separate because of how graceful-fs
  // (and possibly other) modules monkey patch the values.
  get FileReadStream() {
    lazyLoadStreams();
    return FileReadStream;
  },

  set FileReadStream(val) {
    FileReadStream = val;
  },

  get FileWriteStream() {
    lazyLoadStreams();
    return FileWriteStream;
  },

  set FileWriteStream(val) {
    FileWriteStream = val;
  },

  // For tests
  _toUnixTimestamp: toUnixTimestamp,
};

defineLazyProperties(
  fs,
  'internal/fs/dir',
  ['Dir', 'opendir', 'opendirSync'],
);

ObjectDefineProperties(fs, {
  F_OK: { __proto__: null, enumerable: true, value: F_OK || 0 },
  R_OK: { __proto__: null, enumerable: true, value: R_OK || 0 },
  W_OK: { __proto__: null, enumerable: true, value: W_OK || 0 },
  X_OK: { __proto__: null, enumerable: true, value: X_OK || 0 },
  constants: {
    __proto__: null,
    configurable: false,
    enumerable: true,
    value: constants,
  },
  promises: {
    __proto__: null,
    configurable: true,
    enumerable: true,
    get() {
      promises ??= require('internal/fs/promises').exports;
      return promises;
    },
  },
});