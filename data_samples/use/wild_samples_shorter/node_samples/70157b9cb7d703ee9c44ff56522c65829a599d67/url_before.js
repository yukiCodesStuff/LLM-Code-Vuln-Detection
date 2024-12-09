const { encodeStr, hexTable } = require('internal/querystring');

const {
  ERR_INVALID_ARG_TYPE
} = require('internal/errors').codes;
const { validateString } = require('internal/validators');

// This ensures setURLConstructor() is called before the native
  );
}

Url.prototype.parse = function parse(url, parseQueryString, slashesDenoteHost) {
  validateString(url, 'url');

  // Copy chrome, IE, opera backslash-handling behavior.
      this.hostname = this.hostname.toLowerCase();
    }

    if (!ipv6Hostname) {
      // IDNA Support: Returns a punycoded representation of "domain".
      // It only converts parts of the domain name that
      // have non-ASCII characters, i.e. it doesn't matter if
      // you call it with a domain that already is ASCII-only.
      // Use lenient mode (`true`) to try to support even non-compliant
      // URLs.
      this.hostname = toASCII(this.hostname, true);
    }

    const p = this.port ? ':' + this.port : '';
    const h = this.hostname || '';