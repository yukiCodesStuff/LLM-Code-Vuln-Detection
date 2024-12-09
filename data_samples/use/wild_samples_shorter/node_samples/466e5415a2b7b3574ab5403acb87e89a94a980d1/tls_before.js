  ArrayPrototypePush,
  ArrayPrototypeReduce,
  ArrayPrototypeSome,
  ObjectDefineProperty,
  ObjectFreeze,
  RegExpPrototypeTest,
  StringFromCharCode,
  StringPrototypeCharCodeAt,
  StringPrototypeEndsWith,
  StringPrototypeIncludes,
  StringPrototypeReplace,
  StringPrototypeSlice,
  StringPrototypeSplit,
  StringPrototypeStartsWith,
} = primordials;

const {
  ERR_TLS_CERT_ALTNAME_INVALID,
  ERR_OUT_OF_RANGE
} = require('internal/errors').codes;
const internalUtil = require('internal/util');
  return true;
}

exports.checkServerIdentity = function checkServerIdentity(hostname, cert) {
  const subject = cert.subject;
  const altNames = cert.subjectaltname;
  const dnsNames = [];
  hostname = '' + hostname;

  if (altNames) {
    const splitAltNames = StringPrototypeSplit(altNames, ', ');
    ArrayPrototypeForEach(splitAltNames, (name) => {
      if (StringPrototypeStartsWith(name, 'DNS:')) {
        ArrayPrototypePush(dnsNames, StringPrototypeSlice(name, 4));
      } else if (StringPrototypeStartsWith(name, 'URI:')) {