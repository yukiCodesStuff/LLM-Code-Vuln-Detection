  emitInvalidHostnameWarning,
} = require('internal/dns/utils');
const { codes, dnsException } = require('internal/errors');
const { toASCII } = require('internal/idna');
const { isIP, isIPv4, isLegalPort } = require('internal/net');
const {
  getaddrinfo,
  getnameinfo,
    req.resolve = resolve;
    req.reject = reject;

    const err = getaddrinfo(req, toASCII(hostname), family, hints, verbatim);

    if (err) {
      reject(dnsException(err, 'getaddrinfo', hostname));
    }
    req.reject = reject;
    req.ttl = ttl;

    const err = resolver._handle[bindingName](req, toASCII(hostname));

    if (err)
      reject(dnsException(err, bindingName, hostname));
  });