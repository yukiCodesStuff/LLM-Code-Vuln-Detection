const relativeResolveCache = { __proto__: null };

let requireDepth = 0;
let statCache = null;
let isPreloading = false;

function internalRequire(module, id) {
  validateString(id, 'id');
  if (id === '') {
  }
};

// Backwards compatibility
Module.Module = Module;