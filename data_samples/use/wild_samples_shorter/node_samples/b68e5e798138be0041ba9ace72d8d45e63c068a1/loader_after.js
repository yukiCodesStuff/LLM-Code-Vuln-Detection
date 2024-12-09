const relativeResolveCache = { __proto__: null };

let requireDepth = 0;
let isPreloading = false;
let statCache = null;

function internalRequire(module, id) {
  validateString(id, 'id');
  if (id === '') {
  }
};

ObjectDefineProperty(Module.prototype, 'constructor', {
  __proto__: null,
  get: function() {
    return policy() ? undefined : Module;
  },
  configurable: false,
  enumerable: false,
});

// Backwards compatibility
Module.Module = Module;