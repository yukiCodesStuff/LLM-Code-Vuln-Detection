  StringPrototypeStartsWith,
} = primordials;
const {
  ERR_INVALID_ARG_TYPE,
  ERR_MANIFEST_DEPENDENCY_MISSING,
  ERR_UNKNOWN_BUILTIN_MODULE,
} = require('internal/errors').codes;
const { BuiltinModule } = require('internal/bootstrap/realm');
  return mod;
}

let $Module = null;
function lazyModule() {
  $Module = $Module || require('internal/modules/cjs/loader').Module;
  return $Module;
}

// Invoke with makeRequireFunction(module) where |module| is the Module object
// to use as the context for the require() function.
// Use redirects to set up a mapping from a policy and restrict dependencies
const urlToFileCache = new SafeMap();
function makeRequireFunction(mod, redirects) {
  // lazy due to cycle
  const Module = lazyModule();
  if (mod instanceof Module !== true) {
    throw new ERR_INVALID_ARG_TYPE('mod', 'Module', mod);
  }

  let require;
  if (redirects) {
    const id = mod.filename || mod.id;