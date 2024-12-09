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

const getValidatedPath = hideStackFrames((fileURLOrPath, propName = 'path') => {
  const path = toPathIfFileURL(fileURLOrPath);
  validatePath(path, propName);
  return possiblyTransformPath(path);
});

const getValidatedFd = hideStackFrames((fd, propName = 'fd') => {
  if (ObjectIs(fd, -0)) {
    return 0;
  }

  validateInt32(fd, propName, 0);

  return fd;
});

const validateBufferArray = hideStackFrames((buffers, propName = 'buffers') => {
  if (!ArrayIsArray(buffers))
    throw new ERR_INVALID_ARG_TYPE(propName, 'ArrayBufferView[]', buffers);

  for (let i = 0; i < buffers.length; i++) {
    if (!isArrayBufferView(buffers[i]))
      throw new ERR_INVALID_ARG_TYPE(propName, 'ArrayBufferView[]', buffers);
  }

  return buffers;
});

let nonPortableTemplateWarn = true;

function warnOnNonPortableTemplate(template) {
  // Template strings passed to the mkdtemp() family of functions should not
  // end with 'X' because they are handled inconsistently across platforms.
  if (nonPortableTemplateWarn &&
    ((typeof template === 'string' && StringPrototypeEndsWith(template, 'X')) ||
    (typeof template !== 'string' && TypedArrayPrototypeAt(template, -1) === 0x58))) {
    process.emitWarning('mkdtemp() templates ending with X are not portable. ' +
                        'For details see: https://nodejs.org/api/fs.html');
    nonPortableTemplateWarn = false;
  }
}

const defaultCpOptions = {
  dereference: false,
  errorOnExist: false,
  filter: undefined,
  force: true,
  preserveTimestamps: false,
  recursive: false,
  verbatimSymlinks: false,
};

const defaultRmOptions = {
  recursive: false,
  force: false,
  retryDelay: 100,
  maxRetries: 0,
};

const defaultRmdirOptions = {
  retryDelay: 100,
  maxRetries: 0,
  recursive: false,
};

const validateCpOptions = hideStackFrames((options) => {
  if (options === undefined)
    return { ...defaultCpOptions };
  validateObject(options, 'options');
  options = { ...defaultCpOptions, ...options };
  validateBoolean(options.dereference, 'options.dereference');
  validateBoolean(options.errorOnExist, 'options.errorOnExist');
  validateBoolean(options.force, 'options.force');
  validateBoolean(options.preserveTimestamps, 'options.preserveTimestamps');
  validateBoolean(options.recursive, 'options.recursive');
  validateBoolean(options.verbatimSymlinks, 'options.verbatimSymlinks');
  options.mode = getValidMode(options.mode, 'copyFile');
  if (options.dereference === true && options.verbatimSymlinks === true) {
    throw new ERR_INCOMPATIBLE_OPTION_PAIR('dereference', 'verbatimSymlinks');
  }
  if (options.filter !== undefined) {
    validateFunction(options.filter, 'options.filter');
  }
  return options;
});

const validateRmOptions = hideStackFrames((path, options, expectDir, cb) => {
  options = validateRmdirOptions(options, defaultRmOptions);
  validateBoolean(options.force, 'options.force');

  lazyLoadFs().lstat(path, (err, stats) => {
    if (err) {
      if (options.force && err.code === 'ENOENT') {
        return cb(null, options);
      }
      return cb(err, options);
    }

    if (expectDir && !stats.isDirectory()) {
      return cb(false);
    }

    if (stats.isDirectory() && !options.recursive) {
      return cb(new ERR_FS_EISDIR({
        code: 'EISDIR',
        message: 'is a directory',
        path,
        syscall: 'rm',
        errno: EISDIR,
      }));
    }
    return cb(null, options);
  });
});

const validateRmOptionsSync = hideStackFrames((path, options, expectDir) => {
  options = validateRmdirOptions(options, defaultRmOptions);
  validateBoolean(options.force, 'options.force');

  if (!options.force || expectDir || !options.recursive) {
    const isDirectory = lazyLoadFs()
      .lstatSync(path, { throwIfNoEntry: !options.force })?.isDirectory();

    if (expectDir && !isDirectory) {
      return false;
    }

    if (isDirectory && !options.recursive) {
      throw new ERR_FS_EISDIR({
        code: 'EISDIR',
        message: 'is a directory',
        path,
        syscall: 'rm',
        errno: EISDIR,
      });
    }
  }

  return options;
});

let recursiveRmdirWarned = process.noDeprecation;
function emitRecursiveRmdirWarning() {
  if (!recursiveRmdirWarned) {
    process.emitWarning(
      'In future versions of Node.js, fs.rmdir(path, { recursive: true }) ' +
      'will be removed. Use fs.rm(path, { recursive: true }) instead',
      'DeprecationWarning',
      'DEP0147',
    );
    recursiveRmdirWarned = true;
  }
}

const validateRmdirOptions = hideStackFrames(
  (options, defaults = defaultRmdirOptions) => {
    if (options === undefined)
      return defaults;
    validateObject(options, 'options');

    options = { ...defaults, ...options };

    validateBoolean(options.recursive, 'options.recursive');
    validateInt32(options.retryDelay, 'options.retryDelay', 0);
    validateUint32(options.maxRetries, 'options.maxRetries');

    return options;
  });

const getValidMode = hideStackFrames((mode, type) => {
  let min = kMinimumAccessMode;
  let max = kMaximumAccessMode;
  let def = F_OK;
  if (type === 'copyFile') {
    min = kMinimumCopyMode;
    max = kMaximumCopyMode;
    def = mode || kDefaultCopyMode;
  } else {
    assert(type === 'access');
  }
  if (mode == null) {
    return def;
  }
  validateInteger(mode, 'mode', min, max);
  return mode;
});

const validateStringAfterArrayBufferView = hideStackFrames((buffer, name) => {
  if (typeof buffer !== 'string') {
    throw new ERR_INVALID_ARG_TYPE(
      name,
      ['string', 'Buffer', 'TypedArray', 'DataView'],
      buffer,
    );
  }
});

const validatePosition = hideStackFrames((position, name, length) => {
  if (typeof position === 'number') {
    validateInteger(position, name, -1);
  } else if (typeof position === 'bigint') {
    const maxPosition = 2n ** 63n - 1n - BigInt(length);
    if (!(position >= -1n && position <= maxPosition)) {
      throw new ERR_OUT_OF_RANGE(name,
                                 `>= -1 && <= ${maxPosition}`,
                                 position);
    }
  } else {
    throw new ERR_INVALID_ARG_TYPE(name, ['integer', 'bigint'], position);
  }
});

module.exports = {
  constants: {
    kIoMaxLength,
    kMaxUserId,
    kReadFileBufferLength,
    kReadFileUnknownBufferLength,
    kWriteFileMaxChunkSize,
  },
  assertEncoding,
  BigIntStats,  // for testing
  copyObject,
  Dirent,
  emitRecursiveRmdirWarning,
  getDirent,
  getDirents,
  getOptions,
  getValidatedFd,
  getValidatedPath,
  getValidMode,
  handleErrorFromBinding,
  nullCheck,
  possiblyTransformPath,
  preprocessSymlinkDestination,
  realpathCacheKey: Symbol('realpathCacheKey'),
  getStatFsFromBinding,
  getStatsFromBinding,
  stringToFlags,
  stringToSymlinkType,
  Stats,
  toUnixTimestamp,
  validateBufferArray,
  validateCpOptions,
  validateOffsetLengthRead,
  validateOffsetLengthWrite,
  validatePath,
  validatePosition,
  validateRmOptions,
  validateRmOptionsSync,
  validateRmdirOptions,
  validateStringAfterArrayBufferView,
  warnOnNonPortableTemplate,
};