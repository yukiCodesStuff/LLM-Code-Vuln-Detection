const { sep } = path;
const { internalModuleStat } = internalBinding('fs');
const { safeGetenv } = internalBinding('credentials');
const {
  privateSymbols: {
    require_private_symbol,
  },
} = internalBinding('util');
const {
  getCjsConditions,
  initializeCjsConditions,
  hasEsmSyntax,
let statCache = null;
let isPreloading = false;

function internalRequire(module, id) {
  validateString(id, 'id');
  if (id === '') {
    throw new ERR_INVALID_ARG_VALUE('id', id,
                                    'must be a non-empty string');
  }
  requireDepth++;
  try {
    return Module._load(id, module, /* isMain */ false);
  } finally {
    requireDepth--;
  }
}

function stat(filename) {
  filename = path.toNamespacedPath(filename);
  if (statCache !== null) {
    const result = statCache.get(filename);
  this.filename = null;
  this.loaded = false;
  this.children = [];
  let redirects;
  const manifest = policy()?.manifest;
  if (manifest) {
    const moduleURL = pathToFileURL(id);
    redirects = manifest.getDependencyMapper(moduleURL);
  }
  setOwnProperty(this, 'require', makeRequireFunction(this, redirects));
  // Loads a module at the given file path. Returns that module's
  // `exports` property.
  this[require_private_symbol] = internalRequire;
}

Module._cache = { __proto__: null };
Module._pathCache = { __proto__: null };

  if (isMain) {
    process.mainModule = module;
    setOwnProperty(module.require, 'main', process.mainModule);
    module.id = '.';
  }

  reportModuleToWatchMode(filename);
    cascadedLoader.cjsCache.set(this, exports);
};

// Resolved path to process.argv[1] will be lazily placed here
// (needed for setting breakpoint when called with --inspect-brk)
let resolvedArgv;
let hasPausedEntry = false;
// Returns exception, if any.
Module.prototype._compile = function(content, filename) {
  let moduleURL;
  const manifest = policy()?.manifest;
  if (manifest) {
    moduleURL = pathToFileURL(filename);
    manifest.getDependencyMapper(moduleURL);
    manifest.assertIntegrity(moduleURL, content);
  }

  const compiledWrapper = wrapSafe(filename, content, this);
    }
  }
  const dirname = path.dirname(filename);
  let result;
  const exports = this.exports;
  const thisValue = exports;
  const module = this;
  if (requireDepth === 0) statCache = new SafeMap();
  if (inspectorWrapper) {
    result = inspectorWrapper(compiledWrapper, thisValue, exports,
                              module.require, module, filename, dirname);
  } else {
    result = ReflectApply(compiledWrapper, thisValue,
                          [exports, module.require, module, filename, dirname]);
  }
  hasLoadedAnyUserCJSModule = true;
  if (requireDepth === 0) statCache = null;
  return result;
    }
  }
  for (let n = 0; n < requests.length; n++)
    internalRequire(parent, requests[n]);
  isPreloading = false;
};

Module.syncBuiltinESMExports = function syncBuiltinESMExports() {