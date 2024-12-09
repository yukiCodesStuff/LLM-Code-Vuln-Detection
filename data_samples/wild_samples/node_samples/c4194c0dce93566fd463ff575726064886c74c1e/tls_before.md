{
    "name": "AES128-SHA256",
    "standardName": "TLS_RSA_WITH_AES_128_CBC_SHA256",
    "version": "TLSv1.2"
}
```

See
[SSL\_CIPHER\_get\_name](https://www.openssl.org/docs/man1.1.1/man3/SSL_CIPHER_get_name.html)
for more information.

### `tlsSocket.getEphemeralKeyInfo()`

<!-- YAML
added: v5.0.0
-->

* Returns: {Object}

Returns an object representing the type, name, and size of parameter of
an ephemeral key exchange in [perfect forward secrecy][] on a client
connection. It returns an empty object when the key exchange is not
ephemeral. As this is only supported on a client socket; `null` is returned
if called on a server socket. The supported types are `'DH'` and `'ECDH'`. The
`name` property is available only when type is `'ECDH'`.

For example: `{ type: 'ECDH', name: 'prime256v1', size: 256 }`.

### `tlsSocket.getFinished()`

<!-- YAML
added: v9.9.0
-->

* Returns: {Buffer|undefined} The latest `Finished` message that has been
  sent to the socket as part of a SSL/TLS handshake, or `undefined` if
  no `Finished` message has been sent yet.

As the `Finished` messages are message digests of the complete handshake
(with a total of 192 bits for TLS 1.0 and more for SSL 3.0), they can
be used for external authentication procedures when the authentication
provided by SSL/TLS is not desired or is not enough.

Corresponds to the `SSL_get_finished` routine in OpenSSL and may be used
to implement the `tls-unique` channel binding from [RFC 5929][].

### `tlsSocket.getPeerCertificate([detailed])`

<!-- YAML
added: v0.11.4
-->

* `detailed` {boolean} Include the full certificate chain if `true`, otherwise
  include just the peer's certificate.
* Returns: {Object} A certificate object.

Returns an object representing the peer's certificate. If the peer does not
provide a certificate, an empty object will be returned. If the socket has been
destroyed, `null` will be returned.

If the full certificate chain was requested, each certificate will include an
`issuerCertificate` property containing an object representing its issuer's
certificate.

#### Certificate object

<!-- YAML
changes:
  - version: v17.2.0
    pr-url: https://github.com/nodejs/node/pull/39809
    description: Add fingerprint512.
  - version: v11.4.0
    pr-url: https://github.com/nodejs/node/pull/24358
    description: Support Elliptic Curve public key info.
-->

A certificate object has properties corresponding to the fields of the
certificate.

* `raw` {Buffer} The DER encoded X.509 certificate data.
* `subject` {Object} The certificate subject, described in terms of
  Country (`C:`), StateOrProvince (`ST`), Locality (`L`), Organization (`O`),
  OrganizationalUnit (`OU`), and CommonName (`CN`). The CommonName is typically
  a DNS name with TLS certificates. Example:
  `{C: 'UK', ST: 'BC', L: 'Metro', O: 'Node Fans', OU: 'Docs', CN: 'example.com'}`.
* `issuer` {Object} The certificate issuer, described in the same terms as the
  `subject`.
* `valid_from` {string} The date-time the certificate is valid from.
* `valid_to` {string} The date-time the certificate is valid to.
* `serialNumber` {string} The certificate serial number, as a hex string.
  Example: `'B9B0D332A1AA5635'`.
* `fingerprint` {string} The SHA-1 digest of the DER encoded certificate. It is
  returned as a `:` separated hexadecimal string. Example: `'2A:7A:C2:DD:...'`.
* `fingerprint256` {string} The SHA-256 digest of the DER encoded certificate.
  It is returned as a `:` separated hexadecimal string. Example:
  `'2A:7A:C2:DD:...'`.
* `fingerprint512` {string} The SHA-512 digest of the DER encoded certificate.
  It is returned as a `:` separated hexadecimal string. Example:
  `'2A:7A:C2:DD:...'`.
* `ext_key_usage` {Array} (Optional) The extended key usage, a set of OIDs.
* `subjectaltname` {string} (Optional) A string containing concatenated names
  for the subject, an alternative to the `subject` names.
* `infoAccess` {Array} (Optional) An array describing the AuthorityInfoAccess,
  used with OCSP.
* `issuerCertificate` {Object} (Optional) The issuer certificate object. For
  self-signed certificates, this may be a circular reference.

The certificate may contain information about the public key, depending on
the key type.

For RSA keys, the following properties may be defined:

* `bits` {number} The RSA bit size. Example: `1024`.
* `exponent` {string} The RSA exponent, as a string in hexadecimal number
  notation. Example: `'0x010001'`.
* `modulus` {string} The RSA modulus, as a hexadecimal string. Example:
  `'B56CE45CB7...'`.
* `pubkey` {Buffer} The public key.

For EC keys, the following properties may be defined:

* `pubkey` {Buffer} The public key.
* `bits` {number} The key size in bits. Example: `256`.
* `asn1Curve` {string} (Optional) The ASN.1 name of the OID of the elliptic
  curve. Well-known curves are identified by an OID. While it is unusual, it is
  possible that the curve is identified by its mathematical properties, in which
  case it will not have an OID. Example: `'prime256v1'`.
* `nistCurve` {string} (Optional) The NIST name for the elliptic curve, if it
  has one (not all well-known curves have been assigned names by NIST). Example:
  `'P-256'`.

Example certificate:

<!-- eslint-skip -->

```js
{ subject:
   { OU: [ 'Domain Control Validated', 'PositiveSSL Wildcard' ],
     CN: '*.nodejs.org' },
  issuer:
   { C: 'GB',
     ST: 'Greater Manchester',
     L: 'Salford',
     O: 'COMODO CA Limited',
     CN: 'COMODO RSA Domain Validation Secure Server CA' },
  subjectaltname: 'DNS:*.nodejs.org, DNS:nodejs.org',
  infoAccess:
   { 'CA Issuers - URI':
      [ 'http://crt.comodoca.com/COMODORSADomainValidationSecureServerCA.crt' ],
     'OCSP - URI': [ 'http://ocsp.comodoca.com' ] },
  modulus: 'B56CE45CB740B09A13F64AC543B712FF9EE8E4C284B542A1708A27E82A8D151CA178153E12E6DDA15BF70FFD96CB8A88618641BDFCCA03527E665B70D779C8A349A6F88FD4EF6557180BD4C98192872BCFE3AF56E863C09DDD8BC1EC58DF9D94F914F0369102B2870BECFA1348A0838C9C49BD1C20124B442477572347047506B1FCD658A80D0C44BCC16BC5C5496CFE6E4A8428EF654CD3D8972BF6E5BFAD59C93006830B5EB1056BBB38B53D1464FA6E02BFDF2FF66CD949486F0775EC43034EC2602AEFBF1703AD221DAA2A88353C3B6A688EFE8387811F645CEED7B3FE46E1F8B9F59FAD028F349B9BC14211D5830994D055EEA3D547911E07A0ADDEB8A82B9188E58720D95CD478EEC9AF1F17BE8141BE80906F1A339445A7EB5B285F68039B0F294598A7D1C0005FC22B5271B0752F58CCDEF8C8FD856FB7AE21C80B8A2CE983AE94046E53EDE4CB89F42502D31B5360771C01C80155918637490550E3F555E2EE75CC8C636DDE3633CFEDD62E91BF0F7688273694EEEBA20C2FC9F14A2A435517BC1D7373922463409AB603295CEB0BB53787A334C9CA3CA8B30005C5A62FC0715083462E00719A8FA3ED0A9828C3871360A73F8B04A4FC1E71302844E9BB9940B77E745C9D91F226D71AFCAD4B113AAF68D92B24DDB4A2136B55A1CD1ADF39605B63CB639038ED0F4C987689866743A68769CC55847E4A06D6E2E3F1',
  exponent: '0x10001',
  pubkey: <Buffer ... >,
  valid_from: 'Aug 14 00:00:00 2017 GMT',
  valid_to: 'Nov 20 23:59:59 2019 GMT',
  fingerprint: '01:02:59:D9:C3:D2:0D:08:F7:82:4E:44:A4:B4:53:C5:E2:3A:87:4D',
  fingerprint256: '69:AE:1A:6A:D4:3D:C6:C1:1B:EA:C6:23:DE:BA:2A:14:62:62:93:5C:7A:EA:06:41:9B:0B:BC:87:CE:48:4E:02',
  fingerprint512: '19:2B:3E:C3:B3:5B:32:E8:AE:BB:78:97:27:E4:BA:6C:39:C9:92:79:4F:31:46:39:E2:70:E5:5F:89:42:17:C9:E8:64:CA:FF:BB:72:56:73:6E:28:8A:92:7E:A3:2A:15:8B:C2:E0:45:CA:C3:BC:EA:40:52:EC:CA:A2:68:CB:32',
  ext_key_usage: [ '1.3.6.1.5.5.7.3.1', '1.3.6.1.5.5.7.3.2' ],
  serialNumber: '66593D57F20CBC573E433381B5FEC280',
  raw: <Buffer ... > }
```

### `tlsSocket.getPeerFinished()`

<!-- YAML
added: v9.9.0
-->

* Returns: {Buffer|undefined} The latest `Finished` message that is expected
  or has actually been received from the socket as part of a SSL/TLS handshake,
  or `undefined` if there is no `Finished` message so far.

As the `Finished` messages are message digests of the complete handshake
(with a total of 192 bits for TLS 1.0 and more for SSL 3.0), they can
be used for external authentication procedures when the authentication
provided by SSL/TLS is not desired or is not enough.

Corresponds to the `SSL_get_peer_finished` routine in OpenSSL and may be used
to implement the `tls-unique` channel binding from [RFC 5929][].

### `tlsSocket.getPeerX509Certificate()`

<!-- YAML
added: v15.9.0
-->

* Returns: {X509Certificate}

Returns the peer certificate as an {X509Certificate} object.

If there is no peer certificate, or the socket has been destroyed,
`undefined` will be returned.

### `tlsSocket.getProtocol()`

<!-- YAML
added: v5.7.0
-->

* Returns: {string|null}

Returns a string containing the negotiated SSL/TLS protocol version of the
current connection. The value `'unknown'` will be returned for connected
sockets that have not completed the handshaking process. The value `null` will
be returned for server sockets or disconnected client sockets.

Protocol versions are:

* `'SSLv3'`
* `'TLSv1'`
* `'TLSv1.1'`
* `'TLSv1.2'`
* `'TLSv1.3'`

See the OpenSSL [`SSL_get_version`][] documentation for more information.

### `tlsSocket.getSession()`

<!-- YAML
added: v0.11.4
-->

* {Buffer}

Returns the TLS session data or `undefined` if no session was
negotiated. On the client, the data can be provided to the `session` option of
[`tls.connect()`][] to resume the connection. On the server, it may be useful
for debugging.

See [Session Resumption][] for more information.

Note: `getSession()` works only for TLSv1.2 and below. For TLSv1.3, applications
must use the [`'session'`][] event (it also works for TLSv1.2 and below).

### `tlsSocket.getSharedSigalgs()`

<!-- YAML
added: v12.11.0
-->

* Returns: {Array} List of signature algorithms shared between the server and
  the client in the order of decreasing preference.

See
[SSL\_get\_shared\_sigalgs](https://www.openssl.org/docs/man1.1.1/man3/SSL_get_shared_sigalgs.html)
for more information.

### `tlsSocket.getTLSTicket()`

<!-- YAML
added: v0.11.4
-->

* {Buffer}

For a client, returns the TLS session ticket if one is available, or
`undefined`. For a server, always returns `undefined`.

It may be useful for debugging.

See [Session Resumption][] for more information.

### `tlsSocket.getX509Certificate()`

<!-- YAML
added: v15.9.0
-->

* Returns: {X509Certificate}

Returns the local certificate as an {X509Certificate} object.

If there is no local certificate, or the socket has been destroyed,
`undefined` will be returned.

### `tlsSocket.isSessionReused()`

<!-- YAML
added: v0.5.6
-->

* Returns: {boolean} `true` if the session was reused, `false` otherwise.

See [Session Resumption][] for more information.

### `tlsSocket.localAddress`

<!-- YAML
added: v0.11.4
-->

* {string}

Returns the string representation of the local IP address.

### `tlsSocket.localPort`

<!-- YAML
added: v0.11.4
-->

* {number}

Returns the numeric representation of the local port.

### `tlsSocket.remoteAddress`

<!-- YAML
added: v0.11.4
-->

* {string}

Returns the string representation of the remote IP address. For example,
`'74.125.127.100'` or `'2001:4860:a005::68'`.

### `tlsSocket.remoteFamily`

<!-- YAML
added: v0.11.4
-->

* {string}

Returns the string representation of the remote IP family. `'IPv4'` or `'IPv6'`.

### `tlsSocket.remotePort`

<!-- YAML
added: v0.11.4
-->

* {number}

Returns the numeric representation of the remote port. For example, `443`.

### `tlsSocket.renegotiate(options, callback)`

<!-- YAML
added: v0.11.8
-->

* `options` {Object}
  * `rejectUnauthorized` {boolean} If not `false`, the server certificate is
    verified against the list of supplied CAs. An `'error'` event is emitted if
    verification fails; `err.code` contains the OpenSSL error code. **Default:**
    `true`.
  * `requestCert`

* `callback` {Function} If `renegotiate()` returned `true`, callback is
  attached once to the `'secure'` event. If `renegotiate()` returned `false`,
  `callback` will be called in the next tick with an error, unless the
  `tlsSocket` has been destroyed, in which case `callback` will not be called
  at all.

* Returns: {boolean} `true` if renegotiation was initiated, `false` otherwise.

The `tlsSocket.renegotiate()` method initiates a TLS renegotiation process.
Upon completion, the `callback` function will be passed a single argument
that is either an `Error` (if the request failed) or `null`.

This method can be used to request a peer's certificate after the secure
connection has been established.

When running as the server, the socket will be destroyed with an error after
`handshakeTimeout` timeout.

For TLSv1.3, renegotiation cannot be initiated, it is not supported by the
protocol.

### `tlsSocket.setMaxSendFragment(size)`

<!-- YAML
added: v0.11.11
-->

* `size` {number} The maximum TLS fragment size. The maximum value is `16384`.
  **Default:** `16384`.
* Returns: {boolean}

The `tlsSocket.setMaxSendFragment()` method sets the maximum TLS fragment size.
Returns `true` if setting the limit succeeded; `false` otherwise.

Smaller fragment sizes decrease the buffering latency on the client: larger
fragments are buffered by the TLS layer until the entire fragment is received
and its integrity is verified; large fragments can span multiple roundtrips
and their processing can be delayed due to packet loss or reordering. However,
smaller fragments add extra TLS framing bytes and CPU overhead, which may
decrease overall server throughput.

## `tls.checkServerIdentity(hostname, cert)`

<!-- YAML
added: v0.8.4
changes:
  - version:
      - v16.13.2
      - v14.18.3
      - v12.22.9
    pr-url: https://github.com/nodejs-private/node-private/pull/300
    description: Support for `uniformResourceIdentifier` subject alternative
                 names has been disabled in response to CVE-2021-44531.
-->

* `hostname` {string} The host name or IP address to verify the certificate
  against.
* `cert` {Object} A [certificate object][] representing the peer's certificate.
* Returns: {Error|undefined}

Verifies the certificate `cert` is issued to `hostname`.

Returns {Error} object, populating it with `reason`, `host`, and `cert` on
failure. On success, returns {undefined}.

This function can be overwritten by providing an alternative function as the
`options.checkServerIdentity` option that is passed to `tls.connect()`. The
overwriting function can call `tls.checkServerIdentity()` of course, to augment
the checks done with additional verification.

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
changes:
  - version:
      - v15.1.0
      - v14.18.0
    pr-url: https://github.com/nodejs/node/pull/35753
    description: Added `onread` option.
  - version:
      - v14.1.0
      - v13.14.0
    pr-url: https://github.com/nodejs/node/pull/32786
    description: The `highWaterMark` option is accepted now.
  - version:
      - v13.6.0
      - v12.16.0
    pr-url: https://github.com/nodejs/node/pull/23188
    description: The `pskCallback` option is now supported.
  - version: v12.9.0
    pr-url: https://github.com/nodejs/node/pull/27836
    description: Support the `allowHalfOpen` option.
  - version: v12.4.0
    pr-url: https://github.com/nodejs/node/pull/27816
    description: The `hints` option is now supported.
  - version: v12.2.0
    pr-url: https://github.com/nodejs/node/pull/27497
    description: The `enableTrace` option is now supported.
  - version:
      - v11.8.0
      - v10.16.0
    pr-url: https://github.com/nodejs/node/pull/25517
    description: The `timeout` option is supported now.
  - version: v8.0.0
    pr-url: https://github.com/nodejs/node/pull/12839
    description: The `lookup` option is supported now.
  - version: v8.0.0
    pr-url: https://github.com/nodejs/node/pull/11984
    description: The `ALPNProtocols` option can be a `TypedArray` or
     `DataView` now.
  - version:
      - v5.3.0
      - v4.7.0
    pr-url: https://github.com/nodejs/node/pull/4246
    description: The `secureContext` option is supported now.
  - version: v5.0.0
    pr-url: https://github.com/nodejs/node/pull/2564
    description: ALPN options are supported now.
-->

* `options` {Object}
  * `enableTrace`: See [`tls.createServer()`][]
  * `host` {string} Host the client should connect to. **Default:**
    `'localhost'`.
  * `port` {number} Port the client should connect to.
  * `path` {string} Creates Unix socket connection to path. If this option is
    specified, `host` and `port` are ignored.
  * `socket` {stream.Duplex} Establish secure connection on a given socket
    rather than creating a new socket. Typically, this is an instance of
    [`net.Socket`][], but any `Duplex` stream is allowed.
    If this option is specified, `path`, `host` and `port` are ignored,
    except for certificate validation. Usually, a socket is already connected
    when passed to `tls.connect()`, but it can be connected later.
    Connection/disconnection/destruction of `socket` is the user's
    responsibility; calling `tls.connect()` will not cause `net.connect()` to be
    called.
  * `allowHalfOpen` {boolean} If set to `false`, then the socket will
    automatically end the writable side when the readable side ends. If the
    `socket` option is set, this option has no effect. See the `allowHalfOpen`
    option of [`net.Socket`][] for details. **Default:** `false`.
  * `rejectUnauthorized` {boolean} If not `false`, the server certificate is
    verified against the list of supplied CAs. An `'error'` event is emitted if
    verification fails; `err.code` contains the OpenSSL error code. **Default:**
    `true`.
  * `pskCallback` {Function}

    * hint: {string} optional message sent from the server to help client
      decide which identity to use during negotiation.
      Always `null` if TLS 1.3 is used.
    * Returns: {Object} in the form
      `{ psk: <Buffer|TypedArray|DataView>, identity: <string> }`
      or `null` to stop the negotiation process. `psk` must be
      compatible with the selected cipher's digest.
      `identity` must use UTF-8 encoding.

    When negotiating TLS-PSK (pre-shared keys), this function is called
    with optional identity `hint` provided by the server or `null`
    in case of TLS 1.3 where `hint` was removed.
    It will be necessary to provide a custom `tls.checkServerIdentity()`
    for the connection as the default one will try to check host name/IP
    of the server against the certificate but that's not applicable for PSK
    because there won't be a certificate present.
    More information can be found in the [RFC 4279][].
  * `ALPNProtocols`: {string\[]|Buffer\[]|TypedArray\[]|DataView\[]|Buffer|
    TypedArray|DataView}
    An array of strings, `Buffer`s or `TypedArray`s or `DataView`s, or a
    single `Buffer` or `TypedArray` or `DataView` containing the supported ALPN
    protocols. `Buffer`s should have the format `[len][name][len][name]...`
    e.g. `'\x08http/1.1\x08http/1.0'`, where the `len` byte is the length of the
    next protocol name. Passing an array is usually much simpler, e.g.
    `['http/1.1', 'http/1.0']`. Protocols earlier in the list have higher
    preference than those later.
  * `servername`: {string} Server name for the SNI (Server Name Indication) TLS
    extension. It is the name of the host being connected to, and must be a host
    name, and not an IP address. It can be used by a multi-homed server to
    choose the correct certificate to present to the client, see the
    `SNICallback` option to [`tls.createServer()`][].
  * `checkServerIdentity(servername, cert)` {Function} A callback function
    to be used (instead of the builtin `tls.checkServerIdentity()` function)
    when checking the server's host name (or the provided `servername` when
    explicitly set) against the certificate. This should return an {Error} if
    verification fails. The method should return `undefined` if the `servername`
    and `cert` are verified.
  * `session` {Buffer} A `Buffer` instance, containing TLS session.
  * `minDHSize` {number} Minimum size of the DH parameter in bits to accept a
    TLS connection. When a server offers a DH parameter with a size less
    than `minDHSize`, the TLS connection is destroyed and an error is thrown.
    **Default:** `1024`.
  * `highWaterMark`: {number} Consistent with the readable stream `highWaterMark` parameter.
    **Default:** `16 * 1024`.
  * `secureContext`: TLS context object created with
    [`tls.createSecureContext()`][]. If a `secureContext` is _not_ provided, one
    will be created by passing the entire `options` object to
    `tls.createSecureContext()`.
  * `onread` {Object} If the `socket` option is missing, incoming data is
    stored in a single `buffer` and passed to the supplied `callback` when
    data arrives on the socket, otherwise the option is ignored. See the
    `onread` option of [`net.Socket`][] for details.
  * ...: [`tls.createSecureContext()`][] options that are used if the
    `secureContext` option is missing, otherwise they are ignored.
  * ...: Any [`socket.connect()`][] option not already listed.
* `callback` {Function}
* Returns: {tls.TLSSocket}

The `callback` function, if specified, will be added as a listener for the
[`'secureConnect'`][] event.

`tls.connect()` returns a [`tls.TLSSocket`][] object.

Unlike the `https` API, `tls.connect()` does not enable the
SNI (Server Name Indication) extension by default, which may cause some
servers to return an incorrect certificate or reject the connection
altogether. To enable SNI, set the `servername` option in addition
to `host`.

The following illustrates a client for the echo server example from
[`tls.createServer()`][]:

```js
// Assumes an echo server that is listening on port 8000.
const tls = require('tls');
const fs = require('fs');

const options = {
  // Necessary only if the server requires client certificate authentication.
  key: fs.readFileSync('client-key.pem'),
  cert: fs.readFileSync('client-cert.pem'),

  // Necessary only if the server uses a self-signed certificate.
  ca: [ fs.readFileSync('server-cert.pem') ],

  // Necessary only if the server's cert isn't for "localhost".
  checkServerIdentity: () => { return null; },
};

const socket = tls.connect(8000, options, () => {
  console.log('client connected',
              socket.authorized ? 'authorized' : 'unauthorized');
  process.stdin.pipe(socket);
  process.stdin.resume();
});
socket.setEncoding('utf8');
socket.on('data', (data) => {
  console.log(data);
});
socket.on('end', () => {
  console.log('server ends connection');
});
```

## `tls.connect(path[, options][, callback])`

<!-- YAML
added: v0.11.3
-->

* `path` {string} Default value for `options.path`.
* `options` {Object} See [`tls.connect()`][].
* `callback` {Function} See [`tls.connect()`][].
* Returns: {tls.TLSSocket}

Same as [`tls.connect()`][] except that `path` can be provided
as an argument instead of an option.

A path option, if specified, will take precedence over the path argument.

## `tls.connect(port[, host][, options][, callback])`

<!-- YAML
added: v0.11.3
-->

* `port` {number} Default value for `options.port`.
* `host` {string} Default value for `options.host`.
* `options` {Object} See [`tls.connect()`][].
* `callback` {Function} See [`tls.connect()`][].
* Returns: {tls.TLSSocket}

Same as [`tls.connect()`][] except that `port` and `host` can be provided
as arguments instead of options.

A port or host option, if specified, will take precedence over any port or host
argument.

## `tls.createSecureContext([options])`

<!-- YAML
added: v0.11.13
changes:
  - version: v12.12.0
    pr-url: https://github.com/nodejs/node/pull/28973
    description: Added `privateKeyIdentifier` and `privateKeyEngine` options
                 to get private key from an OpenSSL engine.
  - version: v12.11.0
    pr-url: https://github.com/nodejs/node/pull/29598
    description: Added `sigalgs` option to override supported signature
                 algorithms.
  - version: v12.0.0
    pr-url: https://github.com/nodejs/node/pull/26209
    description: TLSv1.3 support added.
  - version: v11.5.0
    pr-url: https://github.com/nodejs/node/pull/24733
    description: The `ca:` option now supports `BEGIN TRUSTED CERTIFICATE`.
  - version:
     - v11.4.0
     - v10.16.0
    pr-url: https://github.com/nodejs/node/pull/24405
    description: The `minVersion` and `maxVersion` can be used to restrict
                 the allowed TLS protocol versions.
  - version: v10.0.0
    pr-url: https://github.com/nodejs/node/pull/19794
    description: The `ecdhCurve` cannot be set to `false` anymore due to a
                 change in OpenSSL.
  - version: v9.3.0
    pr-url: https://github.com/nodejs/node/pull/14903
    description: The `options` parameter can now include `clientCertEngine`.
  - version: v9.0.0
    pr-url: https://github.com/nodejs/node/pull/15206
    description: The `ecdhCurve` option can now be multiple `':'` separated
                 curve names or `'auto'`.
  - version: v7.3.0
    pr-url: https://github.com/nodejs/node/pull/10294
    description: If the `key` option is an array, individual entries do not
                 need a `passphrase` property anymore. `Array` entries can also
                 just be `string`s or `Buffer`s now.
  - version: v5.2.0
    pr-url: https://github.com/nodejs/node/pull/4099
    description: The `ca` option can now be a single string containing multiple
                 CA certificates.
-->

* `options` {Object}
  * `ca` {string|string\[]|Buffer|Buffer\[]} Optionally override the trusted CA
    certificates. Default is to trust the well-known CAs curated by Mozilla.
    Mozilla's CAs are completely replaced when CAs are explicitly specified
    using this option. The value can be a string or `Buffer`, or an `Array` of
    strings and/or `Buffer`s. Any string or `Buffer` can contain multiple PEM
    CAs concatenated together. The peer's certificate must be chainable to a CA
    trusted by the server for the connection to be authenticated. When using
    certificates that are not chainable to a well-known CA, the certificate's CA
    must be explicitly specified as a trusted or the connection will fail to
    authenticate.
    If the peer uses a certificate that doesn't match or chain to one of the
    default CAs, use the `ca` option to provide a CA certificate that the peer's
    certificate can match or chain to.
    For self-signed certificates, the certificate is its own CA, and must be
    provided.
    For PEM encoded certificates, supported types are "TRUSTED CERTIFICATE",
    "X509 CERTIFICATE", and "CERTIFICATE".
    See also [`tls.rootCertificates`][].
  * `cert` {string|string\[]|Buffer|Buffer\[]} Cert chains in PEM format. One
    cert chain should be provided per private key. Each cert chain should
    consist of the PEM formatted certificate for a provided private `key`,
    followed by the PEM formatted intermediate certificates (if any), in order,
    and not including the root CA (the root CA must be pre-known to the peer,
    see `ca`). When providing multiple cert chains, they do not have to be in
    the same order as their private keys in `key`. If the intermediate
    certificates are not provided, the peer will not be able to validate the
    certificate, and the handshake will fail.
  * `sigalgs` {string} Colon-separated list of supported signature algorithms.
    The list can contain digest algorithms (`SHA256`, `MD5` etc.), public key
    algorithms (`RSA-PSS`, `ECDSA` etc.), combination of both (e.g
    'RSA+SHA384') or TLS v1.3 scheme names (e.g. `rsa_pss_pss_sha512`).
    See [OpenSSL man pages](https://www.openssl.org/docs/man1.1.1/man3/SSL_CTX_set1_sigalgs_list.html)
    for more info.
  * `ciphers` {string} Cipher suite specification, replacing the default. For
    more information, see [modifying the default cipher suite][]. Permitted
    ciphers can be obtained via [`tls.getCiphers()`][]. Cipher names must be
    uppercased in order for OpenSSL to accept them.
  * `clientCertEngine` {string} Name of an OpenSSL engine which can provide the
    client certificate.
  * `crl` {string|string\[]|Buffer|Buffer\[]} PEM formatted CRLs (Certificate
    Revocation Lists).
  * `dhparam` {string|Buffer} Diffie-Hellman parameters, required for
    [perfect forward secrecy][]. Use `openssl dhparam` to create the parameters.
    The key length must be greater than or equal to 1024 bits or else an error
    will be thrown. Although 1024 bits is permissible, use 2048 bits or larger
    for stronger security. If omitted or invalid, the parameters are silently
    discarded and DHE ciphers will not be available.
  * `ecdhCurve` {string} A string describing a named curve or a colon separated
    list of curve NIDs or names, for example `P-521:P-384:P-256`, to use for
    ECDH key agreement. Set to `auto` to select the
    curve automatically. Use [`crypto.getCurves()`][] to obtain a list of
    available curve names. On recent releases, `openssl ecparam -list_curves`
    will also display the name and description of each available elliptic curve.
    **Default:** [`tls.DEFAULT_ECDH_CURVE`][].
  * `honorCipherOrder` {boolean} Attempt to use the server's cipher suite
    preferences instead of the client's. When `true`, causes
    `SSL_OP_CIPHER_SERVER_PREFERENCE` to be set in `secureOptions`, see
    [OpenSSL Options][] for more information.
  * `key` {string|string\[]|Buffer|Buffer\[]|Object\[]} Private keys in PEM
    format. PEM allows the option of private keys being encrypted. Encrypted
    keys will be decrypted with `options.passphrase`. Multiple keys using
    different algorithms can be provided either as an array of unencrypted key
    strings or buffers, or an array of objects in the form
    `{pem: <string|buffer>[, passphrase: <string>]}`. The object form can only
    occur in an array. `object.passphrase` is optional. Encrypted keys will be
    decrypted with `object.passphrase` if provided, or `options.passphrase` if
    it is not.
  * `privateKeyEngine` {string} Name of an OpenSSL engine to get private key
    from. Should be used together with `privateKeyIdentifier`.
  * `privateKeyIdentifier` {string} Identifier of a private key managed by
    an OpenSSL engine. Should be used together with `privateKeyEngine`.
    Should not be set together with `key`, because both options define a
    private key in different ways.
  * `maxVersion` {string} Optionally set the maximum TLS version to allow. One
    of `'TLSv1.3'`, `'TLSv1.2'`, `'TLSv1.1'`, or `'TLSv1'`. Cannot be specified
    along with the `secureProtocol` option; use one or the other.
    **Default:** [`tls.DEFAULT_MAX_VERSION`][].
  * `minVersion` {string} Optionally set the minimum TLS version to allow. One
    of `'TLSv1.3'`, `'TLSv1.2'`, `'TLSv1.1'`, or `'TLSv1'`. Cannot be specified
    along with the `secureProtocol` option; use one or the other. Avoid
    setting to less than TLSv1.2, but it may be required for
    interoperability.
    **Default:** [`tls.DEFAULT_MIN_VERSION`][].
  * `passphrase` {string} Shared passphrase used for a single private key and/or
    a PFX.
  * `pfx` {string|string\[]|Buffer|Buffer\[]|Object\[]} PFX or PKCS12 encoded
    private key and certificate chain. `pfx` is an alternative to providing
    `key` and `cert` individually. PFX is usually encrypted, if it is,
    `passphrase` will be used to decrypt it. Multiple PFX can be provided either
    as an array of unencrypted PFX buffers, or an array of objects in the form
    `{buf: <string|buffer>[, passphrase: <string>]}`. The object form can only
    occur in an array. `object.passphrase` is optional. Encrypted PFX will be
    decrypted with `object.passphrase` if provided, or `options.passphrase` if
    it is not.
  * `secureOptions` {number} Optionally affect the OpenSSL protocol behavior,
    which is not usually necessary. This should be used carefully if at all!
    Value is a numeric bitmask of the `SSL_OP_*` options from
    [OpenSSL Options][].
  * `secureProtocol` {string} Legacy mechanism to select the TLS protocol
    version to use, it does not support independent control of the minimum and
    maximum version, and does not support limiting the protocol to TLSv1.3. Use
    `minVersion` and `maxVersion` instead. The possible values are listed as
    [SSL\_METHODS][SSL_METHODS], use the function names as strings. For example,
    use `'TLSv1_1_method'` to force TLS version 1.1, or `'TLS_method'` to allow
    any TLS protocol version up to TLSv1.3. It is not recommended to use TLS
    versions less than 1.2, but it may be required for interoperability.
    **Default:** none, see `minVersion`.
  * `sessionIdContext` {string} Opaque identifier used by servers to ensure
    session state is not shared between applications. Unused by clients.
  * `ticketKeys`: {Buffer} 48-bytes of cryptographically strong pseudorandom
    data. See [Session Resumption][] for more information.
  * `sessionTimeout` {number} The number of seconds after which a TLS session
    created by the server will no longer be resumable. See
    [Session Resumption][] for more information. **Default:** `300`.

[`tls.createServer()`][] sets the default value of the `honorCipherOrder` option
to `true`, other APIs that create secure contexts leave it unset.

[`tls.createServer()`][] uses a 128 bit truncated SHA1 hash value generated
from `process.argv` as the default value of the `sessionIdContext` option, other
APIs that create secure contexts have no default value.

The `tls.createSecureContext()` method creates a `SecureContext` object. It is
usable as an argument to several `tls` APIs, such as [`tls.createServer()`][]
and [`server.addContext()`][], but has no public methods.

A key is _required_ for ciphers that use certificates. Either `key` or
`pfx` can be used to provide it.

If the `ca` option is not given, then Node.js will default to using
[Mozilla's publicly trusted list of CAs][].

## `tls.createSecurePair([context][, isServer][, requestCert][, rejectUnauthorized][, options])`

<!-- YAML
added: v0.3.2
deprecated: v0.11.3
changes:
  - version: v5.0.0
    pr-url: https://github.com/nodejs/node/pull/2564
    description: ALPN options are supported now.
-->

> Stability: 0 - Deprecated: Use [`tls.TLSSocket`][] instead.

* `context` {Object} A secure context object as returned by
  `tls.createSecureContext()`
* `isServer` {boolean} `true` to specify that this TLS connection should be
  opened as a server.
* `requestCert` {boolean} `true` to specify whether a server should request a
  certificate from a connecting client. Only applies when `isServer` is `true`.
* `rejectUnauthorized` {boolean} If not `false` a server automatically reject
  clients with invalid certificates. Only applies when `isServer` is `true`.
* `options`
  * `enableTrace`: See [`tls.createServer()`][]
  * `secureContext`: A TLS context object from [`tls.createSecureContext()`][]
  * `isServer`: If `true` the TLS socket will be instantiated in server-mode.
    **Default:** `false`.
  * `server` {net.Server} A [`net.Server`][] instance
  * `requestCert`: See [`tls.createServer()`][]
  * `rejectUnauthorized`: See [`tls.createServer()`][]
  * `ALPNProtocols`: See [`tls.createServer()`][]
  * `SNICallback`: See [`tls.createServer()`][]
  * `session` {Buffer} A `Buffer` instance containing a TLS session.
  * `requestOCSP` {boolean} If `true`, specifies that the OCSP status request
    extension will be added to the client hello and an `'OCSPResponse'` event
    will be emitted on the socket before establishing a secure communication.

Creates a new secure pair object with two streams, one of which reads and writes
the encrypted data and the other of which reads and writes the cleartext data.
Generally, the encrypted stream is piped to/from an incoming encrypted data
stream and the cleartext one is used as a replacement for the initial encrypted
stream.

`tls.createSecurePair()` returns a `tls.SecurePair` object with `cleartext` and
`encrypted` stream properties.

Using `cleartext` has the same API as [`tls.TLSSocket`][].

The `tls.createSecurePair()` method is now deprecated in favor of
`tls.TLSSocket()`. For example, the code:

```js
pair = tls.createSecurePair(/* ... */);
pair.encrypted.pipe(socket);
socket.pipe(pair.encrypted);
```

can be replaced by:

```js
secureSocket = tls.TLSSocket(socket, options);
```

where `secureSocket` has the same API as `pair.cleartext`.

## `tls.createServer([options][, secureConnectionListener])`

<!-- YAML
added: v0.3.2
changes:
  - version: v12.3.0
    pr-url: https://github.com/nodejs/node/pull/27665
    description: The `options` parameter now supports `net.createServer()`
                 options.
  - version: v9.3.0
    pr-url: https://github.com/nodejs/node/pull/14903
    description: The `options` parameter can now include `clientCertEngine`.
  - version: v8.0.0
    pr-url: https://github.com/nodejs/node/pull/11984
    description: The `ALPNProtocols` option can be a `TypedArray` or
     `DataView` now.
  - version: v5.0.0
    pr-url: https://github.com/nodejs/node/pull/2564
    description: ALPN options are supported now.
-->

* `options` {Object}
  * `ALPNProtocols`: {string\[]|Buffer\[]|TypedArray\[]|DataView\[]|Buffer|
    TypedArray|DataView}
    An array of strings, `Buffer`s or `TypedArray`s or `DataView`s, or a single
    `Buffer` or `TypedArray` or `DataView` containing the supported ALPN
    protocols. `Buffer`s should have the format `[len][name][len][name]...`
    e.g. `0x05hello0x05world`, where the first byte is the length of the next
    protocol name. Passing an array is usually much simpler, e.g.
    `['hello', 'world']`. (Protocols should be ordered by their priority.)
  * `clientCertEngine` {string} Name of an OpenSSL engine which can provide the
    client certificate.
  * `enableTrace` {boolean} If `true`, [`tls.TLSSocket.enableTrace()`][] will be
    called on new connections. Tracing can be enabled after the secure
    connection is established, but this option must be used to trace the secure
    connection setup. **Default:** `false`.
  * `handshakeTimeout` {number} Abort the connection if the SSL/TLS handshake
    does not finish in the specified number of milliseconds.
    A `'tlsClientError'` is emitted on the `tls.Server` object whenever
    a handshake times out. **Default:** `120000` (120 seconds).
  * `rejectUnauthorized` {boolean} If not `false` the server will reject any
    connection which is not authorized with the list of supplied CAs. This
    option only has an effect if `requestCert` is `true`. **Default:** `true`.
  * `requestCert` {boolean} If `true` the server will request a certificate from
    clients that connect and attempt to verify that certificate. **Default:**
    `false`.
  * `sessionTimeout` {number} The number of seconds after which a TLS session
    created by the server will no longer be resumable. See
    [Session Resumption][] for more information. **Default:** `300`.
  * `SNICallback(servername, callback)` {Function} A function that will be
    called if the client supports SNI TLS extension. Two arguments will be
    passed when called: `servername` and `callback`. `callback` is an
    error-first callback that takes two optional arguments: `error` and `ctx`.
    `ctx`, if provided, is a `SecureContext` instance.
    [`tls.createSecureContext()`][] can be used to get a proper `SecureContext`.
    If `callback` is called with a falsy `ctx` argument, the default secure
    context of the server will be used. If `SNICallback` wasn't provided the
    default callback with high-level API will be used (see below).
  * `ticketKeys`: {Buffer} 48-bytes of cryptographically strong pseudorandom
    data. See [Session Resumption][] for more information.
  * `pskCallback` {Function}

    * socket: {tls.TLSSocket} the server [`tls.TLSSocket`][] instance for
      this connection.
    * identity: {string} identity parameter sent from the client.
    * Returns: {Buffer|TypedArray|DataView} pre-shared key that must either be
      a buffer or `null` to stop the negotiation process. Returned PSK must be
      compatible with the selected cipher's digest.

    When negotiating TLS-PSK (pre-shared keys), this function is called
    with the identity provided by the client.
    If the return value is `null` the negotiation process will stop and an
    "unknown\_psk\_identity" alert message will be sent to the other party.
    If the server wishes to hide the fact that the PSK identity was not known,
    the callback must provide some random data as `psk` to make the connection
    fail with "decrypt\_error" before negotiation is finished.
    PSK ciphers are disabled by default, and using TLS-PSK thus
    requires explicitly specifying a cipher suite with the `ciphers` option.
    More information can be found in the [RFC 4279][].
  * `pskIdentityHint` {string} optional hint to send to a client to help
    with selecting the identity during TLS-PSK negotiation. Will be ignored
    in TLS 1.3. Upon failing to set pskIdentityHint `'tlsClientError'` will be
    emitted with `'ERR_TLS_PSK_SET_IDENTIY_HINT_FAILED'` code.
  * ...: Any [`tls.createSecureContext()`][] option can be provided. For
    servers, the identity options (`pfx`, `key`/`cert` or `pskCallback`)
    are usually required.
  * ...: Any [`net.createServer()`][] option can be provided.
* `secureConnectionListener` {Function}
* Returns: {tls.Server}

Creates a new [`tls.Server`][]. The `secureConnectionListener`, if provided, is
automatically set as a listener for the [`'secureConnection'`][] event.

The `ticketKeys` options is automatically shared between `cluster` module
workers.

The following illustrates a simple echo server:

```js
const tls = require('tls');
const fs = require('fs');

const options = {
  key: fs.readFileSync('server-key.pem'),
  cert: fs.readFileSync('server-cert.pem'),

  // This is necessary only if using client certificate authentication.
  requestCert: true,

  // This is necessary only if the client uses a self-signed certificate.
  ca: [ fs.readFileSync('client-cert.pem') ]
};

const server = tls.createServer(options, (socket) => {
  console.log('server connected',
              socket.authorized ? 'authorized' : 'unauthorized');
  socket.write('welcome!\n');
  socket.setEncoding('utf8');
  socket.pipe(socket);
});
server.listen(8000, () => {
  console.log('server bound');
});
```

The server can be tested by connecting to it using the example client from
[`tls.connect()`][].

## `tls.getCiphers()`

<!-- YAML
added: v0.10.2
-->

* Returns: {string\[]}

Returns an array with the names of the supported TLS ciphers. The names are
lower-case for historical reasons, but must be uppercased to be used in
the `ciphers` option of [`tls.createSecureContext()`][].

Cipher names that start with `'tls_'` are for TLSv1.3, all the others are for
TLSv1.2 and below.

```js
console.log(tls.getCiphers()); // ['aes128-gcm-sha256', 'aes128-sha', ...]
```

## `tls.rootCertificates`

<!-- YAML
added: v12.3.0
-->

* {string\[]}

An immutable array of strings representing the root certificates (in PEM format)
from the bundled Mozilla CA store as supplied by current Node.js version.

The bundled CA store, as supplied by Node.js, is a snapshot of Mozilla CA store
that is fixed at release time. It is identical on all supported platforms.

## `tls.DEFAULT_ECDH_CURVE`

<!-- YAML
added: v0.11.13
changes:
  - version: v10.0.0
    pr-url: https://github.com/nodejs/node/pull/16853
    description: Default value changed to `'auto'`.
-->

The default curve name to use for ECDH key agreement in a tls server. The
default value is `'auto'`. See [`tls.createSecureContext()`][] for further
information.

## `tls.DEFAULT_MAX_VERSION`

<!-- YAML
added: v11.4.0
-->

* {string} The default value of the `maxVersion` option of
  [`tls.createSecureContext()`][]. It can be assigned any of the supported TLS
  protocol versions, `'TLSv1.3'`, `'TLSv1.2'`, `'TLSv1.1'`, or `'TLSv1'`.
  **Default:** `'TLSv1.3'`, unless changed using CLI options. Using
  `--tls-max-v1.2` sets the default to `'TLSv1.2'`. Using `--tls-max-v1.3` sets
  the default to `'TLSv1.3'`. If multiple of the options are provided, the
  highest maximum is used.

## `tls.DEFAULT_MIN_VERSION`

<!-- YAML
added: v11.4.0
-->

* {string} The default value of the `minVersion` option of
  [`tls.createSecureContext()`][]. It can be assigned any of the supported TLS
  protocol versions, `'TLSv1.3'`, `'TLSv1.2'`, `'TLSv1.1'`, or `'TLSv1'`.
  **Default:** `'TLSv1.2'`, unless changed using CLI options. Using
  `--tls-min-v1.0` sets the default to `'TLSv1'`. Using `--tls-min-v1.1` sets
  the default to `'TLSv1.1'`. Using `--tls-min-v1.3` sets the default to
  `'TLSv1.3'`. If multiple of the options are provided, the lowest minimum is
  used.

[CVE-2021-44531]: https://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2021-44531
[Chrome's 'modern cryptography' setting]: https://www.chromium.org/Home/chromium-security/education/tls#TOC-Cipher-Suites
[DHE]: https://en.wikipedia.org/wiki/Diffie%E2%80%93Hellman_key_exchange
[ECDHE]: https://en.wikipedia.org/wiki/Elliptic_curve_Diffie%E2%80%93Hellman
[Mozilla's publicly trusted list of CAs]: https://hg.mozilla.org/mozilla-central/raw-file/tip/security/nss/lib/ckfw/builtins/certdata.txt
[OCSP request]: https://en.wikipedia.org/wiki/OCSP_stapling
[OpenSSL Options]: crypto.md#openssl-options
[RFC 2246]: https://www.ietf.org/rfc/rfc2246.txt
[RFC 4086]: https://tools.ietf.org/html/rfc4086
[RFC 4279]: https://tools.ietf.org/html/rfc4279
[RFC 5077]: https://tools.ietf.org/html/rfc5077
[RFC 5929]: https://tools.ietf.org/html/rfc5929
[SSL_METHODS]: https://www.openssl.org/docs/man1.1.1/man7/ssl.html#Dealing-with-Protocol-Methods
[Session Resumption]: #session-resumption
[Stream]: stream.md#stream
[TLS recommendations]: https://wiki.mozilla.org/Security/Server_Side_TLS
[`'newSession'`]: #event-newsession
[`'resumeSession'`]: #event-resumesession
[`'secureConnect'`]: #event-secureconnect
[`'secureConnection'`]: #event-secureconnection
[`'session'`]: #event-session
[`--tls-cipher-list`]: cli.md#--tls-cipher-listlist
[`Duplex`]: stream.md#class-streamduplex
[`NODE_OPTIONS`]: cli.md#node_optionsoptions
[`SSL_export_keying_material`]: https://www.openssl.org/docs/man1.1.1/man3/SSL_export_keying_material.html
[`SSL_get_version`]: https://www.openssl.org/docs/man1.1.1/man3/SSL_get_version.html
[`crypto.getCurves()`]: crypto.md#cryptogetcurves
[`net.Server.address()`]: net.md#serveraddress
[`net.Server`]: net.md#class-netserver
[`net.Socket`]: net.md#class-netsocket
[`net.createServer()`]: net.md#netcreateserveroptions-connectionlistener
[`server.addContext()`]: #serveraddcontexthostname-context
[`server.getTicketKeys()`]: #servergetticketkeys
[`server.listen()`]: net.md#serverlisten
[`server.setTicketKeys()`]: #serversetticketkeyskeys
[`socket.connect()`]: net.md#socketconnectoptions-connectlistener
[`tls.DEFAULT_ECDH_CURVE`]: #tlsdefault_ecdh_curve
[`tls.DEFAULT_MAX_VERSION`]: #tlsdefault_max_version
[`tls.DEFAULT_MIN_VERSION`]: #tlsdefault_min_version
[`tls.Server`]: #class-tlsserver
[`tls.TLSSocket.enableTrace()`]: #tlssocketenabletrace
[`tls.TLSSocket.getPeerCertificate()`]: #tlssocketgetpeercertificatedetailed
[`tls.TLSSocket.getSession()`]: #tlssocketgetsession
[`tls.TLSSocket.getTLSTicket()`]: #tlssocketgettlsticket
[`tls.TLSSocket`]: #class-tlstlssocket
[`tls.connect()`]: #tlsconnectoptions-callback
[`tls.createSecureContext()`]: #tlscreatesecurecontextoptions
[`tls.createSecurePair()`]: #tlscreatesecurepaircontext-isserver-requestcert-rejectunauthorized-options
[`tls.createServer()`]: #tlscreateserveroptions-secureconnectionlistener
[`tls.getCiphers()`]: #tlsgetciphers
[`tls.rootCertificates`]: #tlsrootcertificates
[asn1.js]: https://www.npmjs.com/package/asn1.js
[certificate object]: #certificate-object
[cipher list format]: https://www.openssl.org/docs/man1.1.1/man1/ciphers.html#CIPHER-LIST-FORMAT
[forward secrecy]: https://en.wikipedia.org/wiki/Perfect_forward_secrecy
[modifying the default cipher suite]: #modifying-the-default-tls-cipher-suite
[perfect forward secrecy]: #perfect-forward-secrecy
[specific attacks affecting larger AES key sizes]: https://www.schneier.com/blog/archives/2009/07/another_new_aes.html