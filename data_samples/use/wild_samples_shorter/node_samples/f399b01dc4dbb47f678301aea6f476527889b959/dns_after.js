'use strict';

const cares = internalBinding('cares_wrap');
const { toASCII } = require('internal/idna');
const { isIP, isIPv4, isLegalPort } = require('internal/net');
const { customPromisifyArgs } = require('internal/util');
const errors = require('internal/errors');
const {
  req.hostname = hostname;
  req.oncomplete = all ? onlookupall : onlookup;

  var err = cares.getaddrinfo(req, toASCII(hostname), family, hints, verbatim);
  if (err) {
    process.nextTick(callback, dnsException(err, 'getaddrinfo', hostname));
    return {};
  }
    req.hostname = name;
    req.oncomplete = onresolve;
    req.ttl = !!(options && options.ttl);
    var err = this._handle[bindingName](req, toASCII(name));
    if (err) throw dnsException(err, bindingName, name);
    return req;
  }
  Object.defineProperty(query, 'name', { value: bindingName });