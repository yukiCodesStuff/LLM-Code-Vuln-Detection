
const permission = internalBinding('permission');
const { validateString } = require('internal/validators');
const { isAbsolute, resolve } = require('path');

let experimentalPermission;

module.exports = ObjectFreeze({
      // TODO: add support for WHATWG URLs and Uint8Arrays.
      validateString(reference, 'reference');
      if (StringPrototypeStartsWith(scope, 'fs')) {
        if (!isAbsolute(reference)) {
          reference = resolve(reference);
        }
      }
    }

    return permission.has(scope, reference);