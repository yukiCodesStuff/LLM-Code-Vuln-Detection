  ArrayPrototypePush,
  ArrayPrototypeReduce,
  ArrayPrototypeSome,
  JSONParse,
  ObjectDefineProperty,
  ObjectFreeze,
  RegExpPrototypeExec,
  RegExpPrototypeTest,
  StringFromCharCode,
  StringPrototypeCharCodeAt,
  StringPrototypeEndsWith,
  StringPrototypeIncludes,
  StringPrototypeIndexOf,
  StringPrototypeReplace,
  StringPrototypeSlice,
  StringPrototypeSplit,
  StringPrototypeStartsWith,
  StringPrototypeSubstring,
} = primordials;

const {
  ERR_TLS_CERT_ALTNAME_FORMAT,
  ERR_TLS_CERT_ALTNAME_INVALID,
  ERR_OUT_OF_RANGE
} = require('internal/errors').codes;
const internalUtil = require('internal/util');
  return true;
}

// This pattern is used to determine the length of escaped sequences within
// the subject alt names string. It allows any valid JSON string literal.
// This MUST match the JSON specification (ECMA-404 / RFC8259) exactly.
const jsonStringPattern =
  // eslint-disable-next-line no-control-regex
  /^"(?:[^"\\\u0000-\u001f]|\\(?:["\\/bfnrt]|u[0-9a-fA-F]{4}))*"/;

function splitEscapedAltNames(altNames) {
  const result = [];
  let currentToken = '';
  let offset = 0;
  while (offset !== altNames.length) {
    const nextSep = StringPrototypeIndexOf(altNames, ', ', offset);
    const nextQuote = StringPrototypeIndexOf(altNames, '"', offset);
    if (nextQuote !== -1 && (nextSep === -1 || nextQuote < nextSep)) {
      // There is a quote character and there is no separator before the quote.
      currentToken += StringPrototypeSubstring(altNames, offset, nextQuote);
      const match = RegExpPrototypeExec(
        jsonStringPattern, StringPrototypeSubstring(altNames, nextQuote));
      if (!match) {
        throw new ERR_TLS_CERT_ALTNAME_FORMAT();
      }
      currentToken += JSONParse(match[0]);
      offset = nextQuote + match[0].length;
    } else if (nextSep !== -1) {
      // There is a separator and no quote before it.
      currentToken += StringPrototypeSubstring(altNames, offset, nextSep);
      ArrayPrototypePush(result, currentToken);
      currentToken = '';
      offset = nextSep + 2;
    } else {
      currentToken += StringPrototypeSubstring(altNames, offset);
      offset = altNames.length;
    }
  }
  ArrayPrototypePush(result, currentToken);
  return result;
}

exports.checkServerIdentity = function checkServerIdentity(hostname, cert) {
  const subject = cert.subject;
  const altNames = cert.subjectaltname;
  const dnsNames = [];
  hostname = '' + hostname;

  if (altNames) {
    const splitAltNames = StringPrototypeIncludes(altNames, '"') ?
      splitEscapedAltNames(altNames) :
      StringPrototypeSplit(altNames, ', ');
    ArrayPrototypeForEach(splitAltNames, (name) => {
      if (StringPrototypeStartsWith(name, 'DNS:')) {
        ArrayPrototypePush(dnsNames, StringPrototypeSlice(name, 4));
      } else if (StringPrototypeStartsWith(name, 'URI:')) {