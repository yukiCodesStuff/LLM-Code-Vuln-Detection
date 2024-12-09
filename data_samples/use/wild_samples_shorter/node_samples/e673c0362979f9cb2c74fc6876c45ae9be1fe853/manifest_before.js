  StringPrototypeEndsWith,
  StringPrototypeStartsWith,
  Symbol,
  uncurryThis,
} = primordials;
const {
  ERR_MANIFEST_ASSERT_INTEGRITY,
  ERR_MANIFEST_INVALID_RESOURCE_FIELD,
  debug = fn;
});
const SRI = require('internal/policy/sri');
const crypto = require('crypto');
const { Buffer } = require('buffer');
const { URL } = require('internal/url');
const { createHash, timingSafeEqual } = crypto;
const HashUpdate = uncurryThis(crypto.Hash.prototype.update);
const HashDigest = uncurryThis(crypto.Hash.prototype.digest);
const BufferToString = uncurryThis(Buffer.prototype.toString);
const kRelativeURLStringPattern = /^\.{0,2}\//;
const { getOptionValue } = require('internal/options');
const shouldAbortOnUncaughtException = getOptionValue(
  '--abort-on-uncaught-exception',
        // Avoid clobbered Symbol.iterator
        for (let i = 0; i < integrityEntries.length; i++) {
          const { algorithm, value: expected } = integrityEntries[i];
          const hash = createHash(algorithm);
          // TODO(tniessen): the content should not be passed as a string in the
          // first place, see https://github.com/nodejs/node/issues/39707
          HashUpdate(hash, content, 'utf8');
          const digest = HashDigest(hash, 'buffer');
          if (digest.length === expected.length &&
            timingSafeEqual(digest, expected)) {
            return true;
          }
          realIntegrities.set(algorithm, BufferToString(digest, 'base64'));
        }
      }

      if (integrityEntries !== kCascade) {