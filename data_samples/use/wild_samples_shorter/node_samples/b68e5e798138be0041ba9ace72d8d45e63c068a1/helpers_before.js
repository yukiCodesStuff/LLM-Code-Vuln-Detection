  StringPrototypeStartsWith,
} = primordials;
const {
  ERR_MANIFEST_DEPENDENCY_MISSING,
  ERR_UNKNOWN_BUILTIN_MODULE,
} = require('internal/errors').codes;
const { BuiltinModule } = require('internal/bootstrap/realm');
  return mod;
}

// Invoke with makeRequireFunction(module) where |module| is the Module object
// to use as the context for the require() function.
// Use redirects to set up a mapping from a policy and restrict dependencies
const urlToFileCache = new SafeMap();
function makeRequireFunction(mod, redirects) {
  const Module = mod.constructor;

  let require;
  if (redirects) {
    const id = mod.filename || mod.id;