* `encoding` {string} The [encoding][] of the return value.
* Returns: {Buffer | string}

Generates private and public Diffie-Hellman key values unless they have been
generated or computed already, and returns
the public key in the specified `encoding`. This key should be
transferred to the other party.
If `encoding` is provided a string is returned; otherwise a
[`Buffer`][] is returned.

This function is a thin wrapper around [`DH_generate_key()`][]. In particular,
once a private key has been generated or set, calling this function only updates
the public key but does not generate a new private key.

### `diffieHellman.getGenerator([encoding])`

<!-- YAML
added: v0.5.0
to be a string. If no `encoding` is provided, `privateKey` is expected
to be a [`Buffer`][], `TypedArray`, or `DataView`.

This function does not automatically compute the associated public key. Either
[`diffieHellman.setPublicKey()`][] or [`diffieHellman.generateKeys()`][] can be
used to manually provide the public key or to automatically derive it.

### `diffieHellman.setPublicKey(publicKey[, encoding])`

<!-- YAML
added: v0.5.0
[Web Crypto API documentation]: webcrypto.md
[`BN_is_prime_ex`]: https://www.openssl.org/docs/man1.1.1/man3/BN_is_prime_ex.html
[`Buffer`]: buffer.md
[`DH_generate_key()`]: https://www.openssl.org/docs/man3.0/man3/DH_generate_key.html
[`DiffieHellmanGroup`]: #class-diffiehellmangroup
[`EVP_BytesToKey`]: https://www.openssl.org/docs/man3.0/man3/EVP_BytesToKey.html
[`KeyObject`]: #class-keyobject
[`Sign`]: #class-sign
[`crypto.webcrypto.subtle`]: webcrypto.md#class-subtlecrypto
[`decipher.final()`]: #decipherfinaloutputencoding
[`decipher.update()`]: #decipherupdatedata-inputencoding-outputencoding
[`diffieHellman.generateKeys()`]: #diffiehellmangeneratekeysencoding
[`diffieHellman.setPublicKey()`]: #diffiehellmansetpublickeypublickey-encoding
[`ecdh.generateKeys()`]: #ecdhgeneratekeysencoding-format
[`ecdh.setPrivateKey()`]: #ecdhsetprivatekeyprivatekey-encoding
[`hash.digest()`]: #hashdigestencoding