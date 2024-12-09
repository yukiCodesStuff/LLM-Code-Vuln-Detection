crypto.randomFill(c, (err, buf) => {
  if (err) throw err;
  console.log(Buffer.from(buf.buffer, buf.byteOffset, buf.byteLength)
    .toString('hex'));
});
```

Note that this API uses libuv's threadpool, which can have surprising and
negative performance implications for some applications, see the
[`UV_THREADPOOL_SIZE`][] documentation for more information.

The asynchronous version of `crypto.randomFill()` is carried out in a single
threadpool request. To minimize threadpool task length variation, partition
large `randomFill` requests when doing so as part of fulfilling a client
request.

### crypto.scrypt(password, salt, keylen[, options], callback)
<!-- YAML
added: v10.5.0
changes:
  - version: REPLACEME
    pr-url: https://github.com/nodejs/node/pull/XXX
    description: The `cost`, `blockSize` and `parallelization` option names
                 have been added.
-->
* `password` {string|Buffer|TypedArray|DataView}
* `salt` {string|Buffer|TypedArray|DataView}
* `keylen` {number}
* `options` {Object}
  - `cost` {number} CPU/memory cost parameter. Must be a power of two greater
    than one. **Default:** `16384`.
  - `blockSize` {number} Block size parameter. **Default:** `8`.
  - `parallelization` {number} Parallelization parameter. **Default:** `1`.
  - `N` {number} Alias for `cost`. Only one of both may be specified.
  - `r` {number} Alias for `blockSize`. Only one of both may be specified.
  - `p` {number} Alias for `parallelization`. Only one of both may be specified.
  - `maxmem` {number} Memory upper bound. It is an error when (approximately)
    `128 * N * r > maxmem`. **Default:** `32 * 1024 * 1024`.
* `callback` {Function}
  - `err` {Error}
  - `derivedKey` {Buffer}

Provides an asynchronous [scrypt][] implementation. Scrypt is a password-based
key derivation function that is designed to be expensive computationally and
memory-wise in order to make brute-force attacks unrewarding.

The `salt` should be as unique as possible. It is recommended that a salt is
random and at least 16 bytes long. See [NIST SP 800-132][] for details.

The `callback` function is called with two arguments: `err` and `derivedKey`.
`err` is an exception object when key derivation fails, otherwise `err` is
`null`. `derivedKey` is passed to the callback as a [`Buffer`][].

An exception is thrown when any of the input arguments specify invalid values
or types.

```js
const crypto = require('crypto');
// Using the factory defaults.
crypto.scrypt('secret', 'salt', 64, (err, derivedKey) => {
  if (err) throw err;
  console.log(derivedKey.toString('hex'));  // '3745e48...08d59ae'
});
// Using a custom N parameter. Must be a power of two.
crypto.scrypt('secret', 'salt', 64, { N: 1024 }, (err, derivedKey) => {
  if (err) throw err;
  console.log(derivedKey.toString('hex'));  // '3745e48...aa39b34'
});
```

### crypto.scryptSync(password, salt, keylen[, options])
<!-- YAML
added: v10.5.0
changes:
  - version: REPLACEME
    pr-url: https://github.com/nodejs/node/pull/XXX
    description: The `cost`, `blockSize` and `parallelization` option names
                 have been added.
-->
* `password` {string|Buffer|TypedArray|DataView}
* `salt` {string|Buffer|TypedArray|DataView}
* `keylen` {number}
* `options` {Object}
  - `cost` {number} CPU/memory cost parameter. Must be a power of two greater
    than one. **Default:** `16384`.
  - `blockSize` {number} Block size parameter. **Default:** `8`.
  - `parallelization` {number} Parallelization parameter. **Default:** `1`.
  - `N` {number} Alias for `cost`. Only one of both may be specified.
  - `r` {number} Alias for `blockSize`. Only one of both may be specified.
  - `p` {number} Alias for `parallelization`. Only one of both may be specified.
  - `maxmem` {number} Memory upper bound. It is an error when (approximately)
    `128 * N * r > maxmem`. **Default:** `32 * 1024 * 1024`.
* Returns: {Buffer}

Provides a synchronous [scrypt][] implementation. Scrypt is a password-based
key derivation function that is designed to be expensive computationally and
memory-wise in order to make brute-force attacks unrewarding.

The `salt` should be as unique as possible. It is recommended that a salt is
random and at least 16 bytes long. See [NIST SP 800-132][] for details.

An exception is thrown when key derivation fails, otherwise the derived key is
returned as a [`Buffer`][].

An exception is thrown when any of the input arguments specify invalid values
or types.

```js
const crypto = require('crypto');
// Using the factory defaults.
const key1 = crypto.scryptSync('secret', 'salt', 64);
console.log(key1.toString('hex'));  // '3745e48...08d59ae'
// Using a custom N parameter. Must be a power of two.
const key2 = crypto.scryptSync('secret', 'salt', 64, { N: 1024 });
console.log(key2.toString('hex'));  // '3745e48...aa39b34'
```

### crypto.setEngine(engine[, flags])
<!-- YAML
added: v0.11.11
-->
* `engine` {string}
* `flags` {crypto.constants} **Default:** `crypto.constants.ENGINE_METHOD_ALL`

Load and set the `engine` for some or all OpenSSL functions (selected by flags).

`engine` could be either an id or a path to the engine's shared library.

The optional `flags` argument uses `ENGINE_METHOD_ALL` by default. The `flags`
is a bit field taking one of or a mix of the following flags (defined in
`crypto.constants`):

* `crypto.constants.ENGINE_METHOD_RSA`
* `crypto.constants.ENGINE_METHOD_DSA`
* `crypto.constants.ENGINE_METHOD_DH`
* `crypto.constants.ENGINE_METHOD_RAND`
* `crypto.constants.ENGINE_METHOD_EC`
* `crypto.constants.ENGINE_METHOD_CIPHERS`
* `crypto.constants.ENGINE_METHOD_DIGESTS`
* `crypto.constants.ENGINE_METHOD_PKEY_METHS`
* `crypto.constants.ENGINE_METHOD_PKEY_ASN1_METHS`
* `crypto.constants.ENGINE_METHOD_ALL`
* `crypto.constants.ENGINE_METHOD_NONE`

The flags below are deprecated in OpenSSL-1.1.0.

* `crypto.constants.ENGINE_METHOD_ECDH`
* `crypto.constants.ENGINE_METHOD_ECDSA`
* `crypto.constants.ENGINE_METHOD_STORE`

### crypto.setFips(bool)
<!-- YAML
added: v10.0.0
-->
* `bool` {boolean} `true` to enable FIPS mode.

Enables the FIPS compliant crypto provider in a FIPS-enabled Node.js build.
Throws an error if FIPS mode is not available.

### crypto.timingSafeEqual(a, b)
<!-- YAML
added: v6.6.0
-->
* `a` {Buffer | TypedArray | DataView}
* `b` {Buffer | TypedArray | DataView}
* Returns: {boolean}

This function is based on a constant-time algorithm.
Returns true if `a` is equal to `b`, without leaking timing information that
would allow an attacker to guess one of the values. This is suitable for
comparing HMAC digests or secret values like authentication cookies or
[capability urls](https://www.w3.org/TR/capability-urls/).

`a` and `b` must both be `Buffer`s, `TypedArray`s, or `DataView`s, and they
must have the same length.

Use of `crypto.timingSafeEqual` does not guarantee that the *surrounding* code
is timing-safe. Care should be taken to ensure that the surrounding code does
not introduce timing vulnerabilities.

## Notes

### Legacy Streams API (pre Node.js v0.10)

The Crypto module was added to Node.js before there was the concept of a
unified Stream API, and before there were [`Buffer`][] objects for handling
binary data. As such, the many of the `crypto` defined classes have methods not
typically found on other Node.js classes that implement the [streams][stream]
API (e.g. `update()`, `final()`, or `digest()`). Also, many methods accepted
and returned `'latin1'` encoded strings by default rather than `Buffer`s. This
default was changed after Node.js v0.8 to use [`Buffer`][] objects by default
instead.

### Recent ECDH Changes

Usage of `ECDH` with non-dynamically generated key pairs has been simplified.
Now, [`ecdh.setPrivateKey()`][] can be called with a preselected private key
and the associated public point (key) will be computed and stored in the object.
This allows code to only store and provide the private part of the EC key pair.
[`ecdh.setPrivateKey()`][] now also validates that the private key is valid for
the selected curve.

The [`ecdh.setPublicKey()`][] method is now deprecated as its inclusion in the
API is not useful. Either a previously stored private key should be set, which
automatically generates the associated public key, or [`ecdh.generateKeys()`][]
should be called. The main drawback of using [`ecdh.setPublicKey()`][] is that
it can be used to put the ECDH key pair into an inconsistent state.

### Support for weak or compromised algorithms

The `crypto` module still supports some algorithms which are already
compromised and are not currently recommended for use. The API also allows
the use of ciphers and hashes with a small key size that are considered to be
too weak for safe use.

Users should take full responsibility for selecting the crypto
algorithm and key size according to their security requirements.

Based on the recommendations of [NIST SP 800-131A][]:

- MD5 and SHA-1 are no longer acceptable where collision resistance is
  required such as digital signatures.
- The key used with RSA, DSA, and DH algorithms is recommended to have
  at least 2048 bits and that of the curve of ECDSA and ECDH at least
  224 bits, to be safe to use for several years.
- The DH groups of `modp1`, `modp2` and `modp5` have a key size
  smaller than 2048 bits and are not recommended.

See the reference for other recommendations and details.

### CCM mode

CCM is one of the supported [AEAD algorithms][]. Applications which use this
mode must adhere to certain restrictions when using the cipher API:

- The authentication tag length must be specified during cipher creation by
  setting the `authTagLength` option and must be one of 4, 6, 8, 10, 12, 14 or
  16 bytes.
- The length of the initialization vector (nonce) `N` must be between 7 and 13
  bytes (`7 ≤ N ≤ 13`).
- The length of the plaintext is limited to `2 ** (8 * (15 - N))` bytes.
- When decrypting, the authentication tag must be set via `setAuthTag()` before
  specifying additional authenticated data or calling `update()`.
  Otherwise, decryption will fail and `final()` will throw an error in
  compliance with section 2.6 of [RFC 3610][].
- Using stream methods such as `write(data)`, `end(data)` or `pipe()` in CCM
  mode might fail as CCM cannot handle more than one chunk of data per instance.
- When passing additional authenticated data (AAD), the length of the actual
  message in bytes must be passed to `setAAD()` via the `plaintextLength`
  option. This is not necessary if no AAD is used.
- As CCM processes the whole message at once, `update()` can only be called
  once.
- Even though calling `update()` is sufficient to encrypt/decrypt the message,
  applications *must* call `final()` to compute or verify the
  authentication tag.

```js
const crypto = require('crypto');

const key = 'keykeykeykeykeykeykeykey';
const nonce = crypto.randomBytes(12);

const aad = Buffer.from('0123456789', 'hex');

const cipher = crypto.createCipheriv('aes-192-ccm', key, nonce, {
  authTagLength: 16
});
const plaintext = 'Hello world';
cipher.setAAD(aad, {
  plaintextLength: Buffer.byteLength(plaintext)
});
const ciphertext = cipher.update(plaintext, 'utf8');
cipher.final();
const tag = cipher.getAuthTag();

// Now transmit { ciphertext, nonce, tag }.

const decipher = crypto.createDecipheriv('aes-192-ccm', key, nonce, {
  authTagLength: 16
});
decipher.setAuthTag(tag);
decipher.setAAD(aad, {
  plaintextLength: ciphertext.length
});
const receivedPlaintext = decipher.update(ciphertext, null, 'utf8');

try {
  decipher.final();
} catch (err) {
  console.error('Authentication failed!');
}

console.log(receivedPlaintext);
```

## Crypto Constants

The following constants exported by `crypto.constants` apply to various uses of
the `crypto`, `tls`, and `https` modules and are generally specific to OpenSSL.

### OpenSSL Options
<!--lint disable maximum-line-length-->
<table>
  <tr>
    <th>Constant</th>
    <th>Description</th>
  </tr>
  <tr>
    <td><code>SSL_OP_ALL</code></td>
    <td>Applies multiple bug workarounds within OpenSSL. See
    <a href="https://www.openssl.org/docs/man1.0.2/ssl/SSL_CTX_set_options.html">https://www.openssl.org/docs/man1.0.2/ssl/SSL_CTX_set_options.html</a>
    for detail.</td>
  </tr>
  <tr>
    <td><code>SSL_OP_ALLOW_UNSAFE_LEGACY_RENEGOTIATION</code></td>
    <td>Allows legacy insecure renegotiation between OpenSSL and unpatched
    clients or servers. See
    <a href="https://www.openssl.org/docs/man1.0.2/ssl/SSL_CTX_set_options.html">https://www.openssl.org/docs/man1.0.2/ssl/SSL_CTX_set_options.html</a>.</td>
  </tr>
  <tr>
    <td><code>SSL_OP_CIPHER_SERVER_PREFERENCE</code></td>
    <td>Attempts to use the server's preferences instead of the client's when
    selecting a cipher. Behavior depends on protocol version. See
    <a href="https://www.openssl.org/docs/man1.0.2/ssl/SSL_CTX_set_options.html">https://www.openssl.org/docs/man1.0.2/ssl/SSL_CTX_set_options.html</a>.</td>
  </tr>
  <tr>
    <td><code>SSL_OP_CISCO_ANYCONNECT</code></td>
    <td>Instructs OpenSSL to use Cisco's "speshul" version of DTLS_BAD_VER.</td>
  </tr>
  <tr>
    <td><code>SSL_OP_COOKIE_EXCHANGE</code></td>
    <td>Instructs OpenSSL to turn on cookie exchange.</td>
  </tr>
  <tr>
    <td><code>SSL_OP_CRYPTOPRO_TLSEXT_BUG</code></td>
    <td>Instructs OpenSSL to add server-hello extension from an early version
    of the cryptopro draft.</td>
  </tr>
  <tr>
    <td><code>SSL_OP_DONT_INSERT_EMPTY_FRAGMENTS</code></td>
    <td>Instructs OpenSSL to disable a SSL 3.0/TLS 1.0 vulnerability
    workaround added in OpenSSL 0.9.6d.</td>
  </tr>
  <tr>
    <td><code>SSL_OP_EPHEMERAL_RSA</code></td>
    <td>Instructs OpenSSL to always use the tmp_rsa key when performing RSA
    operations.</td>
  </tr>
  <tr>
    <td><code>SSL_OP_LEGACY_SERVER_CONNECT</code></td>
    <td>Allows initial connection to servers that do not support RI.</td>
  </tr>
  <tr>
    <td><code>SSL_OP_MICROSOFT_BIG_SSLV3_BUFFER</code></td>
    <td></td>
  </tr>
  <tr>
    <td><code>SSL_OP_MICROSOFT_SESS_ID_BUG</code></td>
    <td></td>
  </tr>
  <tr>
    <td><code>SSL_OP_MSIE_SSLV2_RSA_PADDING</code></td>
    <td>Instructs OpenSSL to disable the workaround for a man-in-the-middle
    protocol-version vulnerability in the SSL 2.0 server implementation.</td>
  </tr>
  <tr>
    <td><code>SSL_OP_NETSCAPE_CA_DN_BUG</code></td>
    <td></td>
  </tr>
  <tr>
    <td><code>SSL_OP_NETSCAPE_CHALLENGE_BUG</code></td>
    <td></td>
  </tr>
  <tr>
    <td><code>SSL_OP_NETSCAPE_DEMO_CIPHER_CHANGE_BUG</code></td>
    <td></td>
  </tr>
  <tr>
    <td><code>SSL_OP_NETSCAPE_REUSE_CIPHER_CHANGE_BUG</code></td>
    <td></td>
  </tr>
  <tr>
    <td><code>SSL_OP_NO_COMPRESSION</code></td>
    <td>Instructs OpenSSL to disable support for SSL/TLS compression.</td>
  </tr>
  <tr>
    <td><code>SSL_OP_NO_QUERY_MTU</code></td>
    <td></td>
  </tr>
  <tr>
    <td><code>SSL_OP_NO_SESSION_RESUMPTION_ON_RENEGOTIATION</code></td>
    <td>Instructs OpenSSL to always start a new session when performing
    renegotiation.</td>
  </tr>
  <tr>
    <td><code>SSL_OP_NO_SSLv2</code></td>
    <td>Instructs OpenSSL to turn off SSL v2</td>
  </tr>
  <tr>
    <td><code>SSL_OP_NO_SSLv3</code></td>
    <td>Instructs OpenSSL to turn off SSL v3</td>
  </tr>
  <tr>
    <td><code>SSL_OP_NO_TICKET</code></td>
    <td>Instructs OpenSSL to disable use of RFC4507bis tickets.</td>
  </tr>
  <tr>
    <td><code>SSL_OP_NO_TLSv1</code></td>
    <td>Instructs OpenSSL to turn off TLS v1</td>
  </tr>
  <tr>
    <td><code>SSL_OP_NO_TLSv1_1</code></td>
    <td>Instructs OpenSSL to turn off TLS v1.1</td>
  </tr>
  <tr>
    <td><code>SSL_OP_NO_TLSv1_2</code></td>
    <td>Instructs OpenSSL to turn off TLS v1.2</td>
  </tr>
    <td><code>SSL_OP_PKCS1_CHECK_1</code></td>
    <td></td>
  </tr>
  <tr>
    <td><code>SSL_OP_PKCS1_CHECK_2</code></td>
    <td></td>
  </tr>
  <tr>
    <td><code>SSL_OP_SINGLE_DH_USE</code></td>
    <td>Instructs OpenSSL to always create a new key when using
    temporary/ephemeral DH parameters.</td>
  </tr>
  <tr>
    <td><code>SSL_OP_SINGLE_ECDH_USE</code></td>
    <td>Instructs OpenSSL to always create a new key when using
    temporary/ephemeral ECDH parameters.</td>
  </tr>
    <td><code>SSL_OP_SSLEAY_080_CLIENT_DH_BUG</code></td>
    <td></td>
  </tr>
  <tr>
    <td><code>SSL_OP_SSLREF2_REUSE_CERT_TYPE_BUG</code></td>
    <td></td>
  </tr>
  <tr>
    <td><code>SSL_OP_TLS_BLOCK_PADDING_BUG</code></td>
    <td></td>
  </tr>
  <tr>
    <td><code>SSL_OP_TLS_D5_BUG</code></td>
    <td></td>
  </tr>
  <tr>
    <td><code>SSL_OP_TLS_ROLLBACK_BUG</code></td>
    <td>Instructs OpenSSL to disable version rollback attack detection.</td>
  </tr>
</table>

### OpenSSL Engine Constants
<!--lint enable maximum-line-length remark-lint-->

<table>
  <tr>
    <th>Constant</th>
    <th>Description</th>
  </tr>
  <tr>
    <td><code>ENGINE_METHOD_RSA</code></td>
    <td>Limit engine usage to RSA</td>
  </tr>
  <tr>
    <td><code>ENGINE_METHOD_DSA</code></td>
    <td>Limit engine usage to DSA</td>
  </tr>
  <tr>
    <td><code>ENGINE_METHOD_DH</code></td>
    <td>Limit engine usage to DH</td>
  </tr>
  <tr>
    <td><code>ENGINE_METHOD_RAND</code></td>
    <td>Limit engine usage to RAND</td>
  </tr>
  <tr>
    <td><code>ENGINE_METHOD_EC</code></td>
    <td>Limit engine usage to EC</td>
  </tr>
  <tr>
    <td><code>ENGINE_METHOD_CIPHERS</code></td>
    <td>Limit engine usage to CIPHERS</td>
  </tr>
  <tr>
    <td><code>ENGINE_METHOD_DIGESTS</code></td>
    <td>Limit engine usage to DIGESTS</td>
  </tr>
  <tr>
    <td><code>ENGINE_METHOD_PKEY_METHS</code></td>
    <td>Limit engine usage to PKEY_METHDS</td>
  </tr>
  <tr>
    <td><code>ENGINE_METHOD_PKEY_ASN1_METHS</code></td>
    <td>Limit engine usage to PKEY_ASN1_METHS</td>
  </tr>
  <tr>
    <td><code>ENGINE_METHOD_ALL</code></td>
    <td></td>
  </tr>
  <tr>
    <td><code>ENGINE_METHOD_NONE</code></td>
    <td></td>
  </tr>
</table>

### Other OpenSSL Constants

<table>
  <tr>
    <th>Constant</th>
    <th>Description</th>
  </tr>
  <tr>
    <td><code>DH_CHECK_P_NOT_SAFE_PRIME</code></td>
    <td></td>
  </tr>
  <tr>
    <td><code>DH_CHECK_P_NOT_PRIME</code></td>
    <td></td>
  </tr>
  <tr>
    <td><code>DH_UNABLE_TO_CHECK_GENERATOR</code></td>
    <td></td>
  </tr>
  <tr>
    <td><code>DH_NOT_SUITABLE_GENERATOR</code></td>
    <td></td>
  </tr>
  <tr>
    <td><code>ALPN_ENABLED</code></td>
    <td></td>
  </tr>
  <tr>
    <td><code>RSA_PKCS1_PADDING</code></td>
    <td></td>
  </tr>
  <tr>
    <td><code>RSA_SSLV23_PADDING</code></td>
    <td></td>
  </tr>
  <tr>
    <td><code>RSA_NO_PADDING</code></td>
    <td></td>
  </tr>
  <tr>
    <td><code>RSA_PKCS1_OAEP_PADDING</code></td>
    <td></td>
  </tr>
  <tr>
    <td><code>RSA_X931_PADDING</code></td>
    <td></td>
  </tr>
  <tr>
    <td><code>RSA_PKCS1_PSS_PADDING</code></td>
    <td></td>
  </tr>
  <tr>
    <td><code>RSA_PSS_SALTLEN_DIGEST</code></td>
    <td>Sets the salt length for <code>RSA_PKCS1_PSS_PADDING</code> to the
        digest size when signing or verifying.</td>
  </tr>
  <tr>
    <td><code>RSA_PSS_SALTLEN_MAX_SIGN</code></td>
    <td>Sets the salt length for <code>RSA_PKCS1_PSS_PADDING</code> to the
        maximum permissible value when signing data.</td>
  </tr>
  <tr>
    <td><code>RSA_PSS_SALTLEN_AUTO</code></td>
    <td>Causes the salt length for <code>RSA_PKCS1_PSS_PADDING</code> to be
        determined automatically when verifying a signature.</td>
  </tr>
  <tr>
    <td><code>POINT_CONVERSION_COMPRESSED</code></td>
    <td></td>
  </tr>
  <tr>
    <td><code>POINT_CONVERSION_UNCOMPRESSED</code></td>
    <td></td>
  </tr>
  <tr>
    <td><code>POINT_CONVERSION_HYBRID</code></td>
    <td></td>
  </tr>
</table>

### Node.js Crypto Constants

<table>
  <tr>
    <th>Constant</th>
    <th>Description</th>
  </tr>
  <tr>
    <td><code>defaultCoreCipherList</code></td>
    <td>Specifies the built-in default cipher list used by Node.js.</td>
  </tr>
  <tr>
    <td><code>defaultCipherList</code></td>
    <td>Specifies the active default cipher list used by the current Node.js
    process.</td>
  </tr>
</table>

[`Buffer`]: buffer.html
[`EVP_BytesToKey`]: https://www.openssl.org/docs/man1.1.0/crypto/EVP_BytesToKey.html
[`UV_THREADPOOL_SIZE`]: cli.html#cli_uv_threadpool_size_size
[`cipher.final()`]: #crypto_cipher_final_outputencoding
[`cipher.update()`]: #crypto_cipher_update_data_inputencoding_outputencoding
[`crypto.createCipher()`]: #crypto_crypto_createcipher_algorithm_password_options
[`crypto.createCipheriv()`]: #crypto_crypto_createcipheriv_algorithm_key_iv_options
[`crypto.createDecipher()`]: #crypto_crypto_createdecipher_algorithm_password_options
[`crypto.createDecipheriv()`]: #crypto_crypto_createdecipheriv_algorithm_key_iv_options
[`crypto.createDiffieHellman()`]: #crypto_crypto_creatediffiehellman_prime_primeencoding_generator_generatorencoding
[`crypto.createECDH()`]: #crypto_crypto_createecdh_curvename
[`crypto.createHash()`]: #crypto_crypto_createhash_algorithm_options
[`crypto.createHmac()`]: #crypto_crypto_createhmac_algorithm_key_options
[`crypto.createSign()`]: #crypto_crypto_createsign_algorithm_options
[`crypto.createVerify()`]: #crypto_crypto_createverify_algorithm_options
[`crypto.getCurves()`]: #crypto_crypto_getcurves
[`crypto.getHashes()`]: #crypto_crypto_gethashes
[`crypto.randomBytes()`]: #crypto_crypto_randombytes_size_callback
[`crypto.randomFill()`]: #crypto_crypto_randomfill_buffer_offset_size_callback
[`crypto.scrypt()`]: #crypto_crypto_scrypt_password_salt_keylen_options_callback
[`decipher.final()`]: #crypto_decipher_final_outputencoding
[`decipher.update()`]: #crypto_decipher_update_data_inputencoding_outputencoding
[`diffieHellman.setPublicKey()`]: #crypto_diffiehellman_setpublickey_publickey_encoding
[`ecdh.generateKeys()`]: #crypto_ecdh_generatekeys_encoding_format
[`ecdh.setPrivateKey()`]: #crypto_ecdh_setprivatekey_privatekey_encoding
[`ecdh.setPublicKey()`]: #crypto_ecdh_setpublickey_publickey_encoding
[`hash.digest()`]: #crypto_hash_digest_encoding
[`hash.update()`]: #crypto_hash_update_data_inputencoding
[`hmac.digest()`]: #crypto_hmac_digest_encoding
[`hmac.update()`]: #crypto_hmac_update_data_inputencoding
[`sign.sign()`]: #crypto_sign_sign_privatekey_outputformat
[`sign.update()`]: #crypto_sign_update_data_inputencoding
[`stream.transform` options]: stream.html#stream_new_stream_transform_options
[`stream.Writable` options]: stream.html#stream_constructor_new_stream_writable_options
[`verify.update()`]: #crypto_verify_update_data_inputencoding
[`verify.verify()`]: #crypto_verify_verify_object_signature_signatureformat
[AEAD algorithms]: https://en.wikipedia.org/wiki/Authenticated_encryption
[Caveats]: #crypto_support_for_weak_or_compromised_algorithms
[CCM mode]: #crypto_ccm_mode
[Crypto Constants]: #crypto_crypto_constants_1
[HTML 5.2]: https://www.w3.org/TR/html52/changes.html#features-removed
[HTML5's `keygen` element]: https://developer.mozilla.org/en-US/docs/Web/HTML/Element/keygen
[NIST SP 800-131A]: https://nvlpubs.nist.gov/nistpubs/SpecialPublications/NIST.SP.800-131Ar1.pdf
[NIST SP 800-132]: https://nvlpubs.nist.gov/nistpubs/Legacy/SP/nistspecialpublication800-132.pdf
[NIST SP 800-38D]: https://nvlpubs.nist.gov/nistpubs/Legacy/SP/nistspecialpublication800-38d.pdf
[Nonce-Disrespecting Adversaries]: https://github.com/nonce-disrespect/nonce-disrespect
[OpenSSL's SPKAC implementation]: https://www.openssl.org/docs/man1.1.0/apps/openssl-spkac.html
[RFC 2412]: https://www.rfc-editor.org/rfc/rfc2412.txt
[RFC 3526]: https://www.rfc-editor.org/rfc/rfc3526.txt
[RFC 3610]: https://www.rfc-editor.org/rfc/rfc3610.txt
[RFC 4055]: https://www.rfc-editor.org/rfc/rfc4055.txt
[initialization vector]: https://en.wikipedia.org/wiki/Initialization_vector
[scrypt]: https://en.wikipedia.org/wiki/Scrypt
[stream-writable-write]: stream.html#stream_writable_write_chunk_encoding_callback
[stream]: stream.html
crypto.scrypt('secret', 'salt', 64, { N: 1024 }, (err, derivedKey) => {
  if (err) throw err;
  console.log(derivedKey.toString('hex'));  // '3745e48...aa39b34'
});
```

### crypto.scryptSync(password, salt, keylen[, options])
<!-- YAML
added: v10.5.0
changes:
  - version: REPLACEME
    pr-url: https://github.com/nodejs/node/pull/XXX
    description: The `cost`, `blockSize` and `parallelization` option names
                 have been added.
-->
* `password` {string|Buffer|TypedArray|DataView}
* `salt` {string|Buffer|TypedArray|DataView}
* `keylen` {number}
* `options` {Object}
  - `cost` {number} CPU/memory cost parameter. Must be a power of two greater
    than one. **Default:** `16384`.
  - `blockSize` {number} Block size parameter. **Default:** `8`.
  - `parallelization` {number} Parallelization parameter. **Default:** `1`.
  - `N` {number} Alias for `cost`. Only one of both may be specified.
  - `r` {number} Alias for `blockSize`. Only one of both may be specified.
  - `p` {number} Alias for `parallelization`. Only one of both may be specified.
  - `maxmem` {number} Memory upper bound. It is an error when (approximately)
    `128 * N * r > maxmem`. **Default:** `32 * 1024 * 1024`.
* Returns: {Buffer}

Provides a synchronous [scrypt][] implementation. Scrypt is a password-based
key derivation function that is designed to be expensive computationally and
memory-wise in order to make brute-force attacks unrewarding.

The `salt` should be as unique as possible. It is recommended that a salt is
random and at least 16 bytes long. See [NIST SP 800-132][] for details.

An exception is thrown when key derivation fails, otherwise the derived key is
returned as a [`Buffer`][].

An exception is thrown when any of the input arguments specify invalid values
or types.

```js
const crypto = require('crypto');
// Using the factory defaults.
const key1 = crypto.scryptSync('secret', 'salt', 64);
console.log(key1.toString('hex'));  // '3745e48...08d59ae'
// Using a custom N parameter. Must be a power of two.
const key2 = crypto.scryptSync('secret', 'salt', 64, { N: 1024 });
console.log(key2.toString('hex'));  // '3745e48...aa39b34'
```

### crypto.setEngine(engine[, flags])
<!-- YAML
added: v0.11.11
-->
* `engine` {string}
* `flags` {crypto.constants} **Default:** `crypto.constants.ENGINE_METHOD_ALL`

Load and set the `engine` for some or all OpenSSL functions (selected by flags).

`engine` could be either an id or a path to the engine's shared library.

The optional `flags` argument uses `ENGINE_METHOD_ALL` by default. The `flags`
is a bit field taking one of or a mix of the following flags (defined in
`crypto.constants`):

* `crypto.constants.ENGINE_METHOD_RSA`
* `crypto.constants.ENGINE_METHOD_DSA`
* `crypto.constants.ENGINE_METHOD_DH`
* `crypto.constants.ENGINE_METHOD_RAND`
* `crypto.constants.ENGINE_METHOD_EC`
* `crypto.constants.ENGINE_METHOD_CIPHERS`
* `crypto.constants.ENGINE_METHOD_DIGESTS`
* `crypto.constants.ENGINE_METHOD_PKEY_METHS`
* `crypto.constants.ENGINE_METHOD_PKEY_ASN1_METHS`
* `crypto.constants.ENGINE_METHOD_ALL`
* `crypto.constants.ENGINE_METHOD_NONE`

The flags below are deprecated in OpenSSL-1.1.0.

* `crypto.constants.ENGINE_METHOD_ECDH`
* `crypto.constants.ENGINE_METHOD_ECDSA`
* `crypto.constants.ENGINE_METHOD_STORE`

### crypto.setFips(bool)
<!-- YAML
added: v10.0.0
-->
* `bool` {boolean} `true` to enable FIPS mode.

Enables the FIPS compliant crypto provider in a FIPS-enabled Node.js build.
Throws an error if FIPS mode is not available.

### crypto.timingSafeEqual(a, b)
<!-- YAML
added: v6.6.0
-->
* `a` {Buffer | TypedArray | DataView}
* `b` {Buffer | TypedArray | DataView}
* Returns: {boolean}

This function is based on a constant-time algorithm.
Returns true if `a` is equal to `b`, without leaking timing information that
would allow an attacker to guess one of the values. This is suitable for
comparing HMAC digests or secret values like authentication cookies or
[capability urls](https://www.w3.org/TR/capability-urls/).

`a` and `b` must both be `Buffer`s, `TypedArray`s, or `DataView`s, and they
must have the same length.

Use of `crypto.timingSafeEqual` does not guarantee that the *surrounding* code
is timing-safe. Care should be taken to ensure that the surrounding code does
not introduce timing vulnerabilities.

## Notes

### Legacy Streams API (pre Node.js v0.10)

The Crypto module was added to Node.js before there was the concept of a
unified Stream API, and before there were [`Buffer`][] objects for handling
binary data. As such, the many of the `crypto` defined classes have methods not
typically found on other Node.js classes that implement the [streams][stream]
API (e.g. `update()`, `final()`, or `digest()`). Also, many methods accepted
and returned `'latin1'` encoded strings by default rather than `Buffer`s. This
default was changed after Node.js v0.8 to use [`Buffer`][] objects by default
instead.

### Recent ECDH Changes

Usage of `ECDH` with non-dynamically generated key pairs has been simplified.
Now, [`ecdh.setPrivateKey()`][] can be called with a preselected private key
and the associated public point (key) will be computed and stored in the object.
This allows code to only store and provide the private part of the EC key pair.
[`ecdh.setPrivateKey()`][] now also validates that the private key is valid for
the selected curve.

The [`ecdh.setPublicKey()`][] method is now deprecated as its inclusion in the
API is not useful. Either a previously stored private key should be set, which
automatically generates the associated public key, or [`ecdh.generateKeys()`][]
should be called. The main drawback of using [`ecdh.setPublicKey()`][] is that
it can be used to put the ECDH key pair into an inconsistent state.

### Support for weak or compromised algorithms

The `crypto` module still supports some algorithms which are already
compromised and are not currently recommended for use. The API also allows
the use of ciphers and hashes with a small key size that are considered to be
too weak for safe use.

Users should take full responsibility for selecting the crypto
algorithm and key size according to their security requirements.

Based on the recommendations of [NIST SP 800-131A][]:

- MD5 and SHA-1 are no longer acceptable where collision resistance is
  required such as digital signatures.
- The key used with RSA, DSA, and DH algorithms is recommended to have
  at least 2048 bits and that of the curve of ECDSA and ECDH at least
  224 bits, to be safe to use for several years.
- The DH groups of `modp1`, `modp2` and `modp5` have a key size
  smaller than 2048 bits and are not recommended.

See the reference for other recommendations and details.

### CCM mode

CCM is one of the supported [AEAD algorithms][]. Applications which use this
mode must adhere to certain restrictions when using the cipher API:

- The authentication tag length must be specified during cipher creation by
  setting the `authTagLength` option and must be one of 4, 6, 8, 10, 12, 14 or
  16 bytes.
- The length of the initialization vector (nonce) `N` must be between 7 and 13
  bytes (`7 ≤ N ≤ 13`).
- The length of the plaintext is limited to `2 ** (8 * (15 - N))` bytes.
- When decrypting, the authentication tag must be set via `setAuthTag()` before
  specifying additional authenticated data or calling `update()`.
  Otherwise, decryption will fail and `final()` will throw an error in
  compliance with section 2.6 of [RFC 3610][].
- Using stream methods such as `write(data)`, `end(data)` or `pipe()` in CCM
  mode might fail as CCM cannot handle more than one chunk of data per instance.
- When passing additional authenticated data (AAD), the length of the actual
  message in bytes must be passed to `setAAD()` via the `plaintextLength`
  option. This is not necessary if no AAD is used.
- As CCM processes the whole message at once, `update()` can only be called
  once.
- Even though calling `update()` is sufficient to encrypt/decrypt the message,
  applications *must* call `final()` to compute or verify the
  authentication tag.

```js
const crypto = require('crypto');

const key = 'keykeykeykeykeykeykeykey';
const nonce = crypto.randomBytes(12);

const aad = Buffer.from('0123456789', 'hex');

const cipher = crypto.createCipheriv('aes-192-ccm', key, nonce, {
  authTagLength: 16
});
const plaintext = 'Hello world';
cipher.setAAD(aad, {
  plaintextLength: Buffer.byteLength(plaintext)
});
const ciphertext = cipher.update(plaintext, 'utf8');
cipher.final();
const tag = cipher.getAuthTag();

// Now transmit { ciphertext, nonce, tag }.

const decipher = crypto.createDecipheriv('aes-192-ccm', key, nonce, {
  authTagLength: 16
});
decipher.setAuthTag(tag);
decipher.setAAD(aad, {
  plaintextLength: ciphertext.length
});
const receivedPlaintext = decipher.update(ciphertext, null, 'utf8');

try {
  decipher.final();
} catch (err) {
  console.error('Authentication failed!');
}

console.log(receivedPlaintext);
```

## Crypto Constants

The following constants exported by `crypto.constants` apply to various uses of
the `crypto`, `tls`, and `https` modules and are generally specific to OpenSSL.

### OpenSSL Options
<!--lint disable maximum-line-length-->
<table>
  <tr>
    <th>Constant</th>
    <th>Description</th>
  </tr>
  <tr>
    <td><code>SSL_OP_ALL</code></td>
    <td>Applies multiple bug workarounds within OpenSSL. See
    <a href="https://www.openssl.org/docs/man1.0.2/ssl/SSL_CTX_set_options.html">https://www.openssl.org/docs/man1.0.2/ssl/SSL_CTX_set_options.html</a>
    for detail.</td>
  </tr>
  <tr>
    <td><code>SSL_OP_ALLOW_UNSAFE_LEGACY_RENEGOTIATION</code></td>
    <td>Allows legacy insecure renegotiation between OpenSSL and unpatched
    clients or servers. See
    <a href="https://www.openssl.org/docs/man1.0.2/ssl/SSL_CTX_set_options.html">https://www.openssl.org/docs/man1.0.2/ssl/SSL_CTX_set_options.html</a>.</td>
  </tr>
  <tr>
    <td><code>SSL_OP_CIPHER_SERVER_PREFERENCE</code></td>
    <td>Attempts to use the server's preferences instead of the client's when
    selecting a cipher. Behavior depends on protocol version. See
    <a href="https://www.openssl.org/docs/man1.0.2/ssl/SSL_CTX_set_options.html">https://www.openssl.org/docs/man1.0.2/ssl/SSL_CTX_set_options.html</a>.</td>
  </tr>
  <tr>
    <td><code>SSL_OP_CISCO_ANYCONNECT</code></td>
    <td>Instructs OpenSSL to use Cisco's "speshul" version of DTLS_BAD_VER.</td>
  </tr>
  <tr>
    <td><code>SSL_OP_COOKIE_EXCHANGE</code></td>
    <td>Instructs OpenSSL to turn on cookie exchange.</td>
  </tr>
  <tr>
    <td><code>SSL_OP_CRYPTOPRO_TLSEXT_BUG</code></td>
    <td>Instructs OpenSSL to add server-hello extension from an early version
    of the cryptopro draft.</td>
  </tr>
  <tr>
    <td><code>SSL_OP_DONT_INSERT_EMPTY_FRAGMENTS</code></td>
    <td>Instructs OpenSSL to disable a SSL 3.0/TLS 1.0 vulnerability
    workaround added in OpenSSL 0.9.6d.</td>
  </tr>
  <tr>
    <td><code>SSL_OP_EPHEMERAL_RSA</code></td>
    <td>Instructs OpenSSL to always use the tmp_rsa key when performing RSA
    operations.</td>
  </tr>
  <tr>
    <td><code>SSL_OP_LEGACY_SERVER_CONNECT</code></td>
    <td>Allows initial connection to servers that do not support RI.</td>
  </tr>
  <tr>
    <td><code>SSL_OP_MICROSOFT_BIG_SSLV3_BUFFER</code></td>
    <td></td>
  </tr>
  <tr>
    <td><code>SSL_OP_MICROSOFT_SESS_ID_BUG</code></td>
    <td></td>
  </tr>
  <tr>
    <td><code>SSL_OP_MSIE_SSLV2_RSA_PADDING</code></td>
    <td>Instructs OpenSSL to disable the workaround for a man-in-the-middle
    protocol-version vulnerability in the SSL 2.0 server implementation.</td>
  </tr>
  <tr>
    <td><code>SSL_OP_NETSCAPE_CA_DN_BUG</code></td>
    <td></td>
  </tr>
  <tr>
    <td><code>SSL_OP_NETSCAPE_CHALLENGE_BUG</code></td>
    <td></td>
  </tr>
  <tr>
    <td><code>SSL_OP_NETSCAPE_DEMO_CIPHER_CHANGE_BUG</code></td>
    <td></td>
  </tr>
  <tr>
    <td><code>SSL_OP_NETSCAPE_REUSE_CIPHER_CHANGE_BUG</code></td>
    <td></td>
  </tr>
  <tr>
    <td><code>SSL_OP_NO_COMPRESSION</code></td>
    <td>Instructs OpenSSL to disable support for SSL/TLS compression.</td>
  </tr>
  <tr>
    <td><code>SSL_OP_NO_QUERY_MTU</code></td>
    <td></td>
  </tr>
  <tr>
    <td><code>SSL_OP_NO_SESSION_RESUMPTION_ON_RENEGOTIATION</code></td>
    <td>Instructs OpenSSL to always start a new session when performing
    renegotiation.</td>
  </tr>
  <tr>
    <td><code>SSL_OP_NO_SSLv2</code></td>
    <td>Instructs OpenSSL to turn off SSL v2</td>
  </tr>
  <tr>
    <td><code>SSL_OP_NO_SSLv3</code></td>
    <td>Instructs OpenSSL to turn off SSL v3</td>
  </tr>
  <tr>
    <td><code>SSL_OP_NO_TICKET</code></td>
    <td>Instructs OpenSSL to disable use of RFC4507bis tickets.</td>
  </tr>
  <tr>
    <td><code>SSL_OP_NO_TLSv1</code></td>
    <td>Instructs OpenSSL to turn off TLS v1</td>
  </tr>
  <tr>
    <td><code>SSL_OP_NO_TLSv1_1</code></td>
    <td>Instructs OpenSSL to turn off TLS v1.1</td>
  </tr>
  <tr>
    <td><code>SSL_OP_NO_TLSv1_2</code></td>
    <td>Instructs OpenSSL to turn off TLS v1.2</td>
  </tr>
    <td><code>SSL_OP_PKCS1_CHECK_1</code></td>
    <td></td>
  </tr>
  <tr>
    <td><code>SSL_OP_PKCS1_CHECK_2</code></td>
    <td></td>
  </tr>
  <tr>
    <td><code>SSL_OP_SINGLE_DH_USE</code></td>
    <td>Instructs OpenSSL to always create a new key when using
    temporary/ephemeral DH parameters.</td>
  </tr>
  <tr>
    <td><code>SSL_OP_SINGLE_ECDH_USE</code></td>
    <td>Instructs OpenSSL to always create a new key when using
    temporary/ephemeral ECDH parameters.</td>
  </tr>
    <td><code>SSL_OP_SSLEAY_080_CLIENT_DH_BUG</code></td>
    <td></td>
  </tr>
  <tr>
    <td><code>SSL_OP_SSLREF2_REUSE_CERT_TYPE_BUG</code></td>
    <td></td>
  </tr>
  <tr>
    <td><code>SSL_OP_TLS_BLOCK_PADDING_BUG</code></td>
    <td></td>
  </tr>
  <tr>
    <td><code>SSL_OP_TLS_D5_BUG</code></td>
    <td></td>
  </tr>
  <tr>
    <td><code>SSL_OP_TLS_ROLLBACK_BUG</code></td>
    <td>Instructs OpenSSL to disable version rollback attack detection.</td>
  </tr>
</table>

### OpenSSL Engine Constants
<!--lint enable maximum-line-length remark-lint-->

<table>
  <tr>
    <th>Constant</th>
    <th>Description</th>
  </tr>
  <tr>
    <td><code>ENGINE_METHOD_RSA</code></td>
    <td>Limit engine usage to RSA</td>
  </tr>
  <tr>
    <td><code>ENGINE_METHOD_DSA</code></td>
    <td>Limit engine usage to DSA</td>
  </tr>
  <tr>
    <td><code>ENGINE_METHOD_DH</code></td>
    <td>Limit engine usage to DH</td>
  </tr>
  <tr>
    <td><code>ENGINE_METHOD_RAND</code></td>
    <td>Limit engine usage to RAND</td>
  </tr>
  <tr>
    <td><code>ENGINE_METHOD_EC</code></td>
    <td>Limit engine usage to EC</td>
  </tr>
  <tr>
    <td><code>ENGINE_METHOD_CIPHERS</code></td>
    <td>Limit engine usage to CIPHERS</td>
  </tr>
  <tr>
    <td><code>ENGINE_METHOD_DIGESTS</code></td>
    <td>Limit engine usage to DIGESTS</td>
  </tr>
  <tr>
    <td><code>ENGINE_METHOD_PKEY_METHS</code></td>
    <td>Limit engine usage to PKEY_METHDS</td>
  </tr>
  <tr>
    <td><code>ENGINE_METHOD_PKEY_ASN1_METHS</code></td>
    <td>Limit engine usage to PKEY_ASN1_METHS</td>
  </tr>
  <tr>
    <td><code>ENGINE_METHOD_ALL</code></td>
    <td></td>
  </tr>
  <tr>
    <td><code>ENGINE_METHOD_NONE</code></td>
    <td></td>
  </tr>
</table>

### Other OpenSSL Constants

<table>
  <tr>
    <th>Constant</th>
    <th>Description</th>
  </tr>
  <tr>
    <td><code>DH_CHECK_P_NOT_SAFE_PRIME</code></td>
    <td></td>
  </tr>
  <tr>
    <td><code>DH_CHECK_P_NOT_PRIME</code></td>
    <td></td>
  </tr>
  <tr>
    <td><code>DH_UNABLE_TO_CHECK_GENERATOR</code></td>
    <td></td>
  </tr>
  <tr>
    <td><code>DH_NOT_SUITABLE_GENERATOR</code></td>
    <td></td>
  </tr>
  <tr>
    <td><code>ALPN_ENABLED</code></td>
    <td></td>
  </tr>
  <tr>
    <td><code>RSA_PKCS1_PADDING</code></td>
    <td></td>
  </tr>
  <tr>
    <td><code>RSA_SSLV23_PADDING</code></td>
    <td></td>
  </tr>
  <tr>
    <td><code>RSA_NO_PADDING</code></td>
    <td></td>
  </tr>
  <tr>
    <td><code>RSA_PKCS1_OAEP_PADDING</code></td>
    <td></td>
  </tr>
  <tr>
    <td><code>RSA_X931_PADDING</code></td>
    <td></td>
  </tr>
  <tr>
    <td><code>RSA_PKCS1_PSS_PADDING</code></td>
    <td></td>
  </tr>
  <tr>
    <td><code>RSA_PSS_SALTLEN_DIGEST</code></td>
    <td>Sets the salt length for <code>RSA_PKCS1_PSS_PADDING</code> to the
        digest size when signing or verifying.</td>
  </tr>
  <tr>
    <td><code>RSA_PSS_SALTLEN_MAX_SIGN</code></td>
    <td>Sets the salt length for <code>RSA_PKCS1_PSS_PADDING</code> to the
        maximum permissible value when signing data.</td>
  </tr>
  <tr>
    <td><code>RSA_PSS_SALTLEN_AUTO</code></td>
    <td>Causes the salt length for <code>RSA_PKCS1_PSS_PADDING</code> to be
        determined automatically when verifying a signature.</td>
  </tr>
  <tr>
    <td><code>POINT_CONVERSION_COMPRESSED</code></td>
    <td></td>
  </tr>
  <tr>
    <td><code>POINT_CONVERSION_UNCOMPRESSED</code></td>
    <td></td>
  </tr>
  <tr>
    <td><code>POINT_CONVERSION_HYBRID</code></td>
    <td></td>
  </tr>
</table>

### Node.js Crypto Constants

<table>
  <tr>
    <th>Constant</th>
    <th>Description</th>
  </tr>
  <tr>
    <td><code>defaultCoreCipherList</code></td>
    <td>Specifies the built-in default cipher list used by Node.js.</td>
  </tr>
  <tr>
    <td><code>defaultCipherList</code></td>
    <td>Specifies the active default cipher list used by the current Node.js
    process.</td>
  </tr>
</table>

[`Buffer`]: buffer.html
[`EVP_BytesToKey`]: https://www.openssl.org/docs/man1.1.0/crypto/EVP_BytesToKey.html
[`UV_THREADPOOL_SIZE`]: cli.html#cli_uv_threadpool_size_size
[`cipher.final()`]: #crypto_cipher_final_outputencoding
[`cipher.update()`]: #crypto_cipher_update_data_inputencoding_outputencoding
[`crypto.createCipher()`]: #crypto_crypto_createcipher_algorithm_password_options
[`crypto.createCipheriv()`]: #crypto_crypto_createcipheriv_algorithm_key_iv_options
[`crypto.createDecipher()`]: #crypto_crypto_createdecipher_algorithm_password_options
[`crypto.createDecipheriv()`]: #crypto_crypto_createdecipheriv_algorithm_key_iv_options
[`crypto.createDiffieHellman()`]: #crypto_crypto_creatediffiehellman_prime_primeencoding_generator_generatorencoding
[`crypto.createECDH()`]: #crypto_crypto_createecdh_curvename
[`crypto.createHash()`]: #crypto_crypto_createhash_algorithm_options
[`crypto.createHmac()`]: #crypto_crypto_createhmac_algorithm_key_options
[`crypto.createSign()`]: #crypto_crypto_createsign_algorithm_options
[`crypto.createVerify()`]: #crypto_crypto_createverify_algorithm_options
[`crypto.getCurves()`]: #crypto_crypto_getcurves
[`crypto.getHashes()`]: #crypto_crypto_gethashes
[`crypto.randomBytes()`]: #crypto_crypto_randombytes_size_callback
[`crypto.randomFill()`]: #crypto_crypto_randomfill_buffer_offset_size_callback
[`crypto.scrypt()`]: #crypto_crypto_scrypt_password_salt_keylen_options_callback
[`decipher.final()`]: #crypto_decipher_final_outputencoding
[`decipher.update()`]: #crypto_decipher_update_data_inputencoding_outputencoding
[`diffieHellman.setPublicKey()`]: #crypto_diffiehellman_setpublickey_publickey_encoding
[`ecdh.generateKeys()`]: #crypto_ecdh_generatekeys_encoding_format
[`ecdh.setPrivateKey()`]: #crypto_ecdh_setprivatekey_privatekey_encoding
[`ecdh.setPublicKey()`]: #crypto_ecdh_setpublickey_publickey_encoding
[`hash.digest()`]: #crypto_hash_digest_encoding
[`hash.update()`]: #crypto_hash_update_data_inputencoding
[`hmac.digest()`]: #crypto_hmac_digest_encoding
[`hmac.update()`]: #crypto_hmac_update_data_inputencoding
[`sign.sign()`]: #crypto_sign_sign_privatekey_outputformat
[`sign.update()`]: #crypto_sign_update_data_inputencoding
[`stream.transform` options]: stream.html#stream_new_stream_transform_options
[`stream.Writable` options]: stream.html#stream_constructor_new_stream_writable_options
[`verify.update()`]: #crypto_verify_update_data_inputencoding
[`verify.verify()`]: #crypto_verify_verify_object_signature_signatureformat
[AEAD algorithms]: https://en.wikipedia.org/wiki/Authenticated_encryption
[Caveats]: #crypto_support_for_weak_or_compromised_algorithms
[CCM mode]: #crypto_ccm_mode
[Crypto Constants]: #crypto_crypto_constants_1
[HTML 5.2]: https://www.w3.org/TR/html52/changes.html#features-removed
[HTML5's `keygen` element]: https://developer.mozilla.org/en-US/docs/Web/HTML/Element/keygen
[NIST SP 800-131A]: https://nvlpubs.nist.gov/nistpubs/SpecialPublications/NIST.SP.800-131Ar1.pdf
[NIST SP 800-132]: https://nvlpubs.nist.gov/nistpubs/Legacy/SP/nistspecialpublication800-132.pdf
[NIST SP 800-38D]: https://nvlpubs.nist.gov/nistpubs/Legacy/SP/nistspecialpublication800-38d.pdf
[Nonce-Disrespecting Adversaries]: https://github.com/nonce-disrespect/nonce-disrespect
[OpenSSL's SPKAC implementation]: https://www.openssl.org/docs/man1.1.0/apps/openssl-spkac.html
[RFC 2412]: https://www.rfc-editor.org/rfc/rfc2412.txt
[RFC 3526]: https://www.rfc-editor.org/rfc/rfc3526.txt
[RFC 3610]: https://www.rfc-editor.org/rfc/rfc3610.txt
[RFC 4055]: https://www.rfc-editor.org/rfc/rfc4055.txt
[initialization vector]: https://en.wikipedia.org/wiki/Initialization_vector
[scrypt]: https://en.wikipedia.org/wiki/Scrypt
[stream-writable-write]: stream.html#stream_writable_write_chunk_encoding_callback
[stream]: stream.html