* `encoding` {string} The [encoding][] of the return value.
* Returns: {Buffer | string}

Generates private and public Diffie-Hellman key values, and returns
the public key in the specified `encoding`. This key should be
transferred to the other party.
If `encoding` is provided a string is returned; otherwise a
[`Buffer`][] is returned.

### `diffieHellman.getGenerator([encoding])`

<!-- YAML
added: v0.5.0
to be a string. If no `encoding` is provided, `privateKey` is expected
to be a [`Buffer`][], `TypedArray`, or `DataView`.

### `diffieHellman.setPublicKey(publicKey[, encoding])`

<!-- YAML
added: v0.5.0
[Web Crypto API documentation]: webcrypto.md
[`BN_is_prime_ex`]: https://www.openssl.org/docs/man1.1.1/man3/BN_is_prime_ex.html
[`Buffer`]: buffer.md
[`DiffieHellmanGroup`]: #class-diffiehellmangroup
[`EVP_BytesToKey`]: https://www.openssl.org/docs/man3.0/man3/EVP_BytesToKey.html
[`KeyObject`]: #class-keyobject
[`Sign`]: #class-sign
[`crypto.webcrypto.subtle`]: webcrypto.md#class-subtlecrypto
[`decipher.final()`]: #decipherfinaloutputencoding
[`decipher.update()`]: #decipherupdatedata-inputencoding-outputencoding
[`diffieHellman.setPublicKey()`]: #diffiehellmansetpublickeypublickey-encoding
[`ecdh.generateKeys()`]: #ecdhgeneratekeysencoding-format
[`ecdh.setPrivateKey()`]: #ecdhsetprivatekeyprivatekey-encoding
[`hash.digest()`]: #hashdigestencoding