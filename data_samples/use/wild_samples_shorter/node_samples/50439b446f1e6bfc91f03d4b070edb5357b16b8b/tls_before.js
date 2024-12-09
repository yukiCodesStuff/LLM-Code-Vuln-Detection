const { getOptionValue } = require('internal/options');
const { getRootCertificates, getSSLCiphers } = internalBinding('crypto');
const { Buffer } = require('buffer');
const { URL } = require('internal/url');
const { canonicalizeIP } = internalBinding('cares_wrap');
const _tls_common = require('_tls_common');
const _tls_wrap = require('_tls_wrap');
const { createSecurePair } = require('internal/tls/secure-pair');
  const subject = cert.subject;
  const altNames = cert.subjectaltname;
  const dnsNames = [];
  const uriNames = [];
  const ips = [];

  hostname = '' + hostname;

    ArrayPrototypeForEach(splitAltNames, (name) => {
      if (StringPrototypeStartsWith(name, 'DNS:')) {
        ArrayPrototypePush(dnsNames, StringPrototypeSlice(name, 4));
      } else if (StringPrototypeStartsWith(name, 'URI:')) {
        const uri = new URL(StringPrototypeSlice(name, 4));

        // TODO(bnoordhuis) Also use scheme.
        ArrayPrototypePush(uriNames, uri.hostname);
      } else if (StringPrototypeStartsWith(name, 'IP Address:')) {
        ArrayPrototypePush(ips, canonicalizeIP(StringPrototypeSlice(name, 11)));
      }
    });
  let valid = false;
  let reason = 'Unknown reason';

  const hasAltNames =
    dnsNames.length > 0 || ips.length > 0 || uriNames.length > 0;

  hostname = unfqdn(hostname);  // Remove trailing dot for error messages.

  if (net.isIP(hostname)) {
    valid = ArrayPrototypeIncludes(ips, canonicalizeIP(hostname));
    if (!valid)
      reason = `IP: ${hostname} is not in the cert's list: ` +
               ArrayPrototypeJoin(ips, ', ');
    // TODO(bnoordhuis) Also check URI SANs that are IP addresses.
  } else if (hasAltNames || subject) {
    const hostParts = splitHost(hostname);
    const wildcard = (pattern) => check(hostParts, pattern, true);

    if (hasAltNames) {
      const noWildcard = (pattern) => check(hostParts, pattern, false);
      valid = ArrayPrototypeSome(dnsNames, wildcard) ||
              ArrayPrototypeSome(uriNames, noWildcard);
      if (!valid)
        reason =
          `Host: ${hostname}. is not in the cert's altnames: ${altNames}`;
    } else {
        reason = `Host: ${hostname}. is not cert's CN: ${cn}`;
    }
  } else {
    reason = 'Cert is empty';
  }

  if (!valid) {
    return new ERR_TLS_CERT_ALTNAME_INVALID(reason, hostname, cert);