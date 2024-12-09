process. The error object will have an `err.info` object property with
additional details.

<a id="ERR_TLS_CERT_ALTNAME_FORMAT"></a>

### `ERR_TLS_CERT_ALTNAME_FORMAT`

This error is thrown by `checkServerIdentity` if a user-supplied
`subjectaltname` property violates encoding rules. Certificate objects produced
by Node.js itself always comply with encoding rules and will never cause
this error.

<a id="ERR_TLS_CERT_ALTNAME_INVALID"></a>

### `ERR_TLS_CERT_ALTNAME_INVALID`
