
const permission = internalBinding('permission');
const { validateString } = require('internal/validators');
const { resolve } = require('path');

let experimentalPermission;

module.exports = ObjectFreeze({
      // TODO: add support for WHATWG URLs and Uint8Arrays.
      validateString(reference, 'reference');
      if (StringPrototypeStartsWith(scope, 'fs')) {
        reference = resolve(reference);
      }
    }

    return permission.has(scope, reference);