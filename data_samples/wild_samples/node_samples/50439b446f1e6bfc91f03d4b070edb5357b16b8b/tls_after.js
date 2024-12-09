const {
  ERR_TLS_CERT_ALTNAME_FORMAT,
  ERR_TLS_CERT_ALTNAME_INVALID,
  ERR_OUT_OF_RANGE
} = require('internal/errors').codes;
const internalUtil = require('internal/util');
internalUtil.assertCrypto();
const { isArrayBufferView } = require('internal/util/types');

const net = require('net');
const { getOptionValue } = require('internal/options');
const { getRootCertificates, getSSLCiphers } = internalBinding('crypto');
const { Buffer } = require('buffer');
const { canonicalizeIP } = internalBinding('cares_wrap');
const _tls_common = require('_tls_common');
const _tls_wrap = require('_tls_wrap');
const { createSecurePair } = require('internal/tls/secure-pair');
const { parseCertString } = require('internal/tls/parse-cert-string');

// Allow {CLIENT_RENEG_LIMIT} client-initiated session renegotiations
// every {CLIENT_RENEG_WINDOW} seconds. An error event is emitted if more
// renegotiations are seen. The settings are applied to all remote client
// connections.
exports.CLIENT_RENEG_LIMIT = 3;
exports.CLIENT_RENEG_WINDOW = 600;

exports.DEFAULT_CIPHERS = getOptionValue('--tls-cipher-list');

exports.DEFAULT_ECDH_CURVE = 'auto';

if (getOptionValue('--tls-min-v1.0'))
  exports.DEFAULT_MIN_VERSION = 'TLSv1';
else if (getOptionValue('--tls-min-v1.1'))
  exports.DEFAULT_MIN_VERSION = 'TLSv1.1';
else if (getOptionValue('--tls-min-v1.2'))
  exports.DEFAULT_MIN_VERSION = 'TLSv1.2';
else if (getOptionValue('--tls-min-v1.3'))
  exports.DEFAULT_MIN_VERSION = 'TLSv1.3';
else
  exports.DEFAULT_MIN_VERSION = 'TLSv1.2';

if (getOptionValue('--tls-max-v1.3'))
  exports.DEFAULT_MAX_VERSION = 'TLSv1.3';
else if (getOptionValue('--tls-max-v1.2'))
  exports.DEFAULT_MAX_VERSION = 'TLSv1.2';
else
  exports.DEFAULT_MAX_VERSION = 'TLSv1.3'; // Will depend on node version.


exports.getCiphers = internalUtil.cachedResult(
  () => internalUtil.filterDuplicateStrings(getSSLCiphers(), true)
);

let rootCertificates;

function cacheRootCertificates() {
  rootCertificates = ObjectFreeze(getRootCertificates());
}
exports.checkServerIdentity = function checkServerIdentity(hostname, cert) {
  const subject = cert.subject;
  const altNames = cert.subjectaltname;
  const dnsNames = [];
  const ips = [];

  hostname = '' + hostname;

  if (altNames) {
    const splitAltNames = StringPrototypeIncludes(altNames, '"') ?
      splitEscapedAltNames(altNames) :
      StringPrototypeSplit(altNames, ', ');
    ArrayPrototypeForEach(splitAltNames, (name) => {
      if (StringPrototypeStartsWith(name, 'DNS:')) {
        ArrayPrototypePush(dnsNames, StringPrototypeSlice(name, 4));
      } else if (StringPrototypeStartsWith(name, 'IP Address:')) {
        ArrayPrototypePush(ips, canonicalizeIP(StringPrototypeSlice(name, 11)));
      }
    });
  }

  let valid = false;
  let reason = 'Unknown reason';

  hostname = unfqdn(hostname);  // Remove trailing dot for error messages.

  if (net.isIP(hostname)) {
    valid = ArrayPrototypeIncludes(ips, canonicalizeIP(hostname));
    if (!valid)
      reason = `IP: ${hostname} is not in the cert's list: ` +
               ArrayPrototypeJoin(ips, ', ');
  } else if (dnsNames.length > 0 || subject?.CN) {
    const hostParts = splitHost(hostname);
    const wildcard = (pattern) => check(hostParts, pattern, true);

    if (dnsNames.length > 0) {
      valid = ArrayPrototypeSome(dnsNames, wildcard);
      if (!valid)
        reason =
          `Host: ${hostname}. is not in the cert's altnames: ${altNames}`;
    } else {
      // Match against Common Name only if no supported identifiers exist.
      const cn = subject.CN;

      if (ArrayIsArray(cn))
        valid = ArrayPrototypeSome(cn, wildcard);
      else if (cn)
        valid = wildcard(cn);

      if (!valid)
        reason = `Host: ${hostname}. is not cert's CN: ${cn}`;
    }
  } else {
    reason = 'Cert does not contain a DNS name';
  }

  if (!valid) {
    return new ERR_TLS_CERT_ALTNAME_INVALID(reason, hostname, cert);
  }
};

exports.createSecureContext = _tls_common.createSecureContext;
exports.SecureContext = _tls_common.SecureContext;
exports.TLSSocket = _tls_wrap.TLSSocket;
exports.Server = _tls_wrap.Server;
exports.createServer = _tls_wrap.createServer;
exports.connect = _tls_wrap.connect;

exports.parseCertString = internalUtil.deprecate(
  parseCertString,
  'tls.parseCertString() is deprecated. ' +
  'Please use querystring.parse() instead.',
  'DEP0076');

exports.createSecurePair = internalUtil.deprecate(
  createSecurePair,
  'tls.createSecurePair() is deprecated. Please use ' +
  'tls.TLSSocket instead.', 'DEP0064');
      if (StringPrototypeStartsWith(name, 'DNS:')) {
        ArrayPrototypePush(dnsNames, StringPrototypeSlice(name, 4));
      } else if (StringPrototypeStartsWith(name, 'IP Address:')) {
        ArrayPrototypePush(ips, canonicalizeIP(StringPrototypeSlice(name, 11)));
      }
    });
  }
      } else if (StringPrototypeStartsWith(name, 'IP Address:')) {
        ArrayPrototypePush(ips, canonicalizeIP(StringPrototypeSlice(name, 11)));
      }
    });
  }

  let valid = false;
  let reason = 'Unknown reason';

  hostname = unfqdn(hostname);  // Remove trailing dot for error messages.

  if (net.isIP(hostname)) {
    valid = ArrayPrototypeIncludes(ips, canonicalizeIP(hostname));
    if (!valid)
      reason = `IP: ${hostname} is not in the cert's list: ` +
               ArrayPrototypeJoin(ips, ', ');
  } else if (dnsNames.length > 0 || subject?.CN) {
    const hostParts = splitHost(hostname);
    const wildcard = (pattern) => check(hostParts, pattern, true);

    if (dnsNames.length > 0) {
      valid = ArrayPrototypeSome(dnsNames, wildcard);
      if (!valid)
        reason =
          `Host: ${hostname}. is not in the cert's altnames: ${altNames}`;
    } else {
      // Match against Common Name only if no supported identifiers exist.
      const cn = subject.CN;

      if (ArrayIsArray(cn))
        valid = ArrayPrototypeSome(cn, wildcard);
      else if (cn)
        valid = wildcard(cn);

      if (!valid)
        reason = `Host: ${hostname}. is not cert's CN: ${cn}`;
    }
  } else {
    reason = 'Cert does not contain a DNS name';
  }

  if (!valid) {
    return new ERR_TLS_CERT_ALTNAME_INVALID(reason, hostname, cert);
  }
};

exports.createSecureContext = _tls_common.createSecureContext;
exports.SecureContext = _tls_common.SecureContext;
exports.TLSSocket = _tls_wrap.TLSSocket;
exports.Server = _tls_wrap.Server;
exports.createServer = _tls_wrap.createServer;
exports.connect = _tls_wrap.connect;

exports.parseCertString = internalUtil.deprecate(
  parseCertString,
  'tls.parseCertString() is deprecated. ' +
  'Please use querystring.parse() instead.',
  'DEP0076');

exports.createSecurePair = internalUtil.deprecate(
  createSecurePair,
  'tls.createSecurePair() is deprecated. Please use ' +
  'tls.TLSSocket instead.', 'DEP0064');
    } else {
      // Match against Common Name only if no supported identifiers exist.
      const cn = subject.CN;

      if (ArrayIsArray(cn))
        valid = ArrayPrototypeSome(cn, wildcard);
      else if (cn)
        valid = wildcard(cn);

      if (!valid)
        reason = `Host: ${hostname}. is not cert's CN: ${cn}`;
    }
  } else {
    reason = 'Cert does not contain a DNS name';
  }

  if (!valid) {
    return new ERR_TLS_CERT_ALTNAME_INVALID(reason, hostname, cert);
  }
};

exports.createSecureContext = _tls_common.createSecureContext;
exports.SecureContext = _tls_common.SecureContext;
exports.TLSSocket = _tls_wrap.TLSSocket;
exports.Server = _tls_wrap.Server;
exports.createServer = _tls_wrap.createServer;
exports.connect = _tls_wrap.connect;

exports.parseCertString = internalUtil.deprecate(
  parseCertString,
  'tls.parseCertString() is deprecated. ' +
  'Please use querystring.parse() instead.',
  'DEP0076');

exports.createSecurePair = internalUtil.deprecate(
  createSecurePair,
  'tls.createSecurePair() is deprecated. Please use ' +
  'tls.TLSSocket instead.', 'DEP0064');