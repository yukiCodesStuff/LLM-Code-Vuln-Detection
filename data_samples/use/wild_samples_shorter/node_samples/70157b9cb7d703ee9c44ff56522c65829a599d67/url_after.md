<!-- YAML
added: v0.1.25
changes:
  - version: REPLACEME
    pr-url: https://github.com/nodejs/node/pull/38631
    description: Now throws an `ERR_INVALID_URL` exception when Punycode
                 conversion of a hostname introduces changes that could cause
                 the URL to be re-parsed differently.
  - version:
      - v15.13.0
      - v14.17.0
    pr-url: https://github.com/nodejs/node/pull/37784