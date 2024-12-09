} = primordials;

const {
  ERR_ACCESS_DENIED,
  ERR_MANIFEST_TDZ,
} = require('internal/errors').codes;
const { Manifest } = require('internal/policy/manifest');
let manifest;
      return o;
    });
    manifest = new Manifest(json, url);

    // process.binding() is deprecated (DEP0111) and trivially allows bypassing
    // policies, so if policies are enabled, make this API unavailable.
    process.binding = function binding(_module) {
      throw new ERR_ACCESS_DENIED('process.binding');
    };
    process._linkedBinding = function _linkedBinding(_module) {
      throw new ERR_ACCESS_DENIED('process._linkedBinding');
    };
  },

  get manifest() {
    if (typeof manifest === 'undefined') {