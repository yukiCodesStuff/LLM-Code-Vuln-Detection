  StringPrototypeEndsWith,
  StringPrototypeStartsWith,
  Symbol,
} = primordials;
const {
  ERR_MANIFEST_ASSERT_INTEGRITY,
  ERR_MANIFEST_INVALID_RESOURCE_FIELD,
  debug = fn;
});
const SRI = require('internal/policy/sri');
const { URL } = require('internal/url');
const { internalVerifyIntegrity } = internalBinding('crypto');
const kRelativeURLStringPattern = /^\.{0,2}\//;
const { getOptionValue } = require('internal/options');
const shouldAbortOnUncaughtException = getOptionValue(
  '--abort-on-uncaught-exception',
        // Avoid clobbered Symbol.iterator
        for (let i = 0; i < integrityEntries.length; i++) {
          const { algorithm, value: expected } = integrityEntries[i];
          // TODO(tniessen): the content should not be passed as a string in the
          // first place, see https://github.com/nodejs/node/issues/39707
          const mismatchedIntegrity = internalVerifyIntegrity(algorithm, content, expected);
          if (mismatchedIntegrity === undefined) {
            return true;
          }
          realIntegrities.set(algorithm, mismatchedIntegrity);
        }
      }

      if (integrityEntries !== kCascade) {