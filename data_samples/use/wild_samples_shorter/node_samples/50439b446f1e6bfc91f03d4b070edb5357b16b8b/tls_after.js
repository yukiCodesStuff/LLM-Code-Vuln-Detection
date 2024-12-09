const { getOptionValue } = require('internal/options');
const { getRootCertificates, getSSLCiphers } = internalBinding('crypto');
const { Buffer } = require('buffer');
const { canonicalizeIP } = internalBinding('cares_wrap');
const _tls_common = require('_tls_common');
const _tls_wrap = require('_tls_wrap');
const { createSecurePair } = require('internal/tls/secure-pair');
  const subject = cert.subject;
  const altNames = cert.subjectaltname;
  const dnsNames = [];
  const ips = [];

  hostname = '' + hostname;

    ArrayPrototypeForEach(splitAltNames, (name) => {
      if (StringPrototypeStartsWith(name, 'DNS:')) {
        ArrayPrototypePush(dnsNames, StringPrototypeSlice(name, 4));
      } else if (StringPrototypeStartsWith(name, 'IP Address:')) {
        ArrayPrototypePush(ips, canonicalizeIP(StringPrototypeSlice(name, 11)));
      }
    });
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
        reason = `Host: ${hostname}. is not cert's CN: ${cn}`;
    }
  } else {
    reason = 'Cert does not contain a DNS name';
  }

  if (!valid) {
    return new ERR_TLS_CERT_ALTNAME_INVALID(reason, hostname, cert);