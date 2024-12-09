
<!-- YAML
added: v0.8.4
changes:
  - version: REPLACEME
    pr-url: https://github.com/nodejs-private/node-private/pull/300
    description: Support for `uniformResourceIdentifier` subject alternative
                 names has been disabled in response to CVE-2021-44531.
-->

* `hostname` {string} The host name or IP address to verify the certificate
  against.
This function is only called if the certificate passed all other checks, such as
being issued by trusted CA (`options.ca`).

Earlier versions of Node.js incorrectly accepted certificates for a given
`hostname` if a matching `uniformResourceIdentifier` subject alternative name
was present (see [CVE-2021-44531][]). Applications that wish to accept
`uniformResourceIdentifier` subject alternative names can use a custom
`options.checkServerIdentity` function that implements the desired behavior.

## `tls.connect(options[, callback])`

<!-- YAML
added: v0.11.3
  `'TLSv1.3'`. If multiple of the options are provided, the lowest minimum is
  used.

[CVE-2021-44531]: https://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2021-44531
[Chrome's 'modern cryptography' setting]: https://www.chromium.org/Home/chromium-security/education/tls#TOC-Cipher-Suites
[DHE]: https://en.wikipedia.org/wiki/Diffie%E2%80%93Hellman_key_exchange
[ECDHE]: https://en.wikipedia.org/wiki/Elliptic_curve_Diffie%E2%80%93Hellman
[Mozilla's publicly trusted list of CAs]: https://hg.mozilla.org/mozilla-central/raw-file/tip/security/nss/lib/ckfw/builtins/certdata.txt