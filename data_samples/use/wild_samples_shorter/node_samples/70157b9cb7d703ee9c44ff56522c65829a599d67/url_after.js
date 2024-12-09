const { encodeStr, hexTable } = require('internal/querystring');

const {
  ERR_INVALID_ARG_TYPE,
  ERR_INVALID_URL,
} = require('internal/errors').codes;
const { validateString } = require('internal/validators');

// This ensures setURLConstructor() is called before the native
  );
}

// This prevents some common spoofing bugs due to our use of IDNA toASCII. For
// compatibility, the set of characters we use here is the *intersection* of
// "forbidden host code point" in the WHATWG URL Standard [1] and the
// characters in the host parsing loop in Url.prototype.parse, with the
// following additions:
//
// - ':' since this could cause a "protocol spoofing" bug
// - '@' since this could cause parts of the hostname to be confused with auth
// - '[' and ']' since this could cause a non-IPv6 hostname to be interpreted
//   as IPv6 by isIpv6Hostname above
//
// [1]: https://url.spec.whatwg.org/#forbidden-host-code-point
const forbiddenHostChars = /[\t\n\r #%/:<>?@[\\\]^|]/;

Url.prototype.parse = function parse(url, parseQueryString, slashesDenoteHost) {
  validateString(url, 'url');

  // Copy chrome, IE, opera backslash-handling behavior.
      this.hostname = this.hostname.toLowerCase();
    }

    if (!ipv6Hostname && this.hostname !== '') {
      // IDNA Support: Returns a punycoded representation of "domain".
      // It only converts parts of the domain name that
      // have non-ASCII characters, i.e. it doesn't matter if
      // you call it with a domain that already is ASCII-only.
      // Use lenient mode (`true`) to try to support even non-compliant
      // URLs.
      this.hostname = toASCII(this.hostname, true);

      // Prevent two potential routes of hostname spoofing.
      // 1. If this.hostname is empty, it must have become empty due to toASCII
      //    since we checked this.hostname above.
      // 2. If any of forbiddenHostChars appears in this.hostname, it must have
      //    also gotten in due to toASCII. This is since getHostname would have
      //    filtered them out otherwise.
      // Rather than trying to correct this by moving the non-host part into
      // the pathname as we've done in getHostname, throw an exception to
      // convey the severity of this issue.
      if (this.hostname === '' || forbiddenHostChars.test(this.hostname)) {
        throw new ERR_INVALID_URL(url);
      }
    }

    const p = this.port ? ':' + this.port : '';
    const h = this.hostname || '';