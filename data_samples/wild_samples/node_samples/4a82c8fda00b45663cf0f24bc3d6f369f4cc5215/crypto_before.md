const {
  createDiffieHellman,
} = require('node:crypto');

// Generate Alice's keys...
const alice = createDiffieHellman(2048);
const aliceKey = alice.generateKeys();

// Generate Bob's keys...
const bob = createDiffieHellman(alice.getPrime(), alice.getGenerator());
const bobKey = bob.generateKeys();

// Exchange and generate the secret...
const aliceSecret = alice.computeSecret(bobKey);
const bobSecret = bob.computeSecret(aliceKey);

// OK
assert.strictEqual(aliceSecret.toString('hex'), bobSecret.toString('hex'));
```

### `diffieHellman.computeSecret(otherPublicKey[, inputEncoding][, outputEncoding])`

<!-- YAML
added: v0.5.0
-->

* `otherPublicKey` {string|ArrayBuffer|Buffer|TypedArray|DataView}
* `inputEncoding` {string} The [encoding][] of an `otherPublicKey` string.
* `outputEncoding` {string} The [encoding][] of the return value.
* Returns: {Buffer | string}

Computes the shared secret using `otherPublicKey` as the other
party's public key and returns the computed shared secret. The supplied
key is interpreted using the specified `inputEncoding`, and secret is
encoded using specified `outputEncoding`.
If the `inputEncoding` is not
provided, `otherPublicKey` is expected to be a [`Buffer`][],
`TypedArray`, or `DataView`.

If `outputEncoding` is given a string is returned; otherwise, a
[`Buffer`][] is returned.

### `diffieHellman.generateKeys([encoding])`

<!-- YAML
added: v0.5.0
-->

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
-->

* `encoding` {string} The [encoding][] of the return value.
* Returns: {Buffer | string}

Returns the Diffie-Hellman generator in the specified `encoding`.
If `encoding` is provided a string is
returned; otherwise a [`Buffer`][] is returned.

### `diffieHellman.getPrime([encoding])`

<!-- YAML
added: v0.5.0
-->

* `encoding` {string} The [encoding][] of the return value.
* Returns: {Buffer | string}

Returns the Diffie-Hellman prime in the specified `encoding`.
If `encoding` is provided a string is
returned; otherwise a [`Buffer`][] is returned.

### `diffieHellman.getPrivateKey([encoding])`

<!-- YAML
added: v0.5.0
-->

* `encoding` {string} The [encoding][] of the return value.
* Returns: {Buffer | string}

Returns the Diffie-Hellman private key in the specified `encoding`.
If `encoding` is provided a
string is returned; otherwise a [`Buffer`][] is returned.

### `diffieHellman.getPublicKey([encoding])`

<!-- YAML
added: v0.5.0
-->

* `encoding` {string} The [encoding][] of the return value.
* Returns: {Buffer | string}

Returns the Diffie-Hellman public key in the specified `encoding`.
If `encoding` is provided a
string is returned; otherwise a [`Buffer`][] is returned.

### `diffieHellman.setPrivateKey(privateKey[, encoding])`

<!-- YAML
added: v0.5.0
-->

* `privateKey` {string|ArrayBuffer|Buffer|TypedArray|DataView}
* `encoding` {string} The [encoding][] of the `privateKey` string.

Sets the Diffie-Hellman private key. If the `encoding` argument is provided,
`privateKey` is expected
to be a string. If no `encoding` is provided, `privateKey` is expected
to be a [`Buffer`][], `TypedArray`, or `DataView`.

### `diffieHellman.setPublicKey(publicKey[, encoding])`

<!-- YAML
added: v0.5.0
-->

* `publicKey` {string|ArrayBuffer|Buffer|TypedArray|DataView}
* `encoding` {string} The [encoding][] of the `publicKey` string.

Sets the Diffie-Hellman public key. If the `encoding` argument is provided,
`publicKey` is expected
to be a string. If no `encoding` is provided, `publicKey` is expected
to be a [`Buffer`][], `TypedArray`, or `DataView`.

### `diffieHellman.verifyError`

<!-- YAML
added: v0.11.12
-->

A bit field containing any warnings and/or errors resulting from a check
performed during initialization of the `DiffieHellman` object.

The following values are valid for this property (as defined in `node:constants` module):

* `DH_CHECK_P_NOT_SAFE_PRIME`
* `DH_CHECK_P_NOT_PRIME`
* `DH_UNABLE_TO_CHECK_GENERATOR`
* `DH_NOT_SUITABLE_GENERATOR`

## Class: `DiffieHellmanGroup`

<!-- YAML
added: v0.7.5
-->

The `DiffieHellmanGroup` class takes a well-known modp group as its argument.
It works the same as `DiffieHellman`, except that it does not allow changing
its keys after creation. In other words, it does not implement `setPublicKey()`
or `setPrivateKey()` methods.

```mjs
const { createDiffieHellmanGroup } = await import('node:crypto');
const dh = createDiffieHellmanGroup('modp16');
```

```cjs
const { createDiffieHellmanGroup } = require('node:crypto');
const dh = createDiffieHellmanGroup('modp16');
```

The following groups are supported:

* `'modp14'` (2048 bits, [RFC 3526][] Section 3)
* `'modp15'` (3072 bits, [RFC 3526][] Section 4)
* `'modp16'` (4096 bits, [RFC 3526][] Section 5)
* `'modp17'` (6144 bits, [RFC 3526][] Section 6)
* `'modp18'` (8192 bits, [RFC 3526][] Section 7)

The following groups are still supported but deprecated (see [Caveats][]):

* `'modp1'` (768 bits, [RFC 2409][] Section 6.1) <span class="deprecated-inline"></span>
* `'modp2'` (1024 bits, [RFC 2409][] Section 6.2) <span class="deprecated-inline"></span>
* `'modp5'` (1536 bits, [RFC 3526][] Section 2) <span class="deprecated-inline"></span>

These deprecated groups might be removed in future versions of Node.js.

## Class: `ECDH`

<!-- YAML
added: v0.11.14
-->

The `ECDH` class is a utility for creating Elliptic Curve Diffie-Hellman (ECDH)
key exchanges.

Instances of the `ECDH` class can be created using the
[`crypto.createECDH()`][] function.

```mjs
import assert from 'node:assert';

const {
  createECDH,
} = await import('node:crypto');

// Generate Alice's keys...
const alice = createECDH('secp521r1');
const aliceKey = alice.generateKeys();

// Generate Bob's keys...
const bob = createECDH('secp521r1');
const bobKey = bob.generateKeys();

// Exchange and generate the secret...
const aliceSecret = alice.computeSecret(bobKey);
const bobSecret = bob.computeSecret(aliceKey);

assert.strictEqual(aliceSecret.toString('hex'), bobSecret.toString('hex'));
// OK
```

```cjs
const assert = require('node:assert');

const {
  createECDH,
} = require('node:crypto');

// Generate Alice's keys...
const alice = createECDH('secp521r1');
const aliceKey = alice.generateKeys();

// Generate Bob's keys...
const bob = createECDH('secp521r1');
const bobKey = bob.generateKeys();

// Exchange and generate the secret...
const aliceSecret = alice.computeSecret(bobKey);
const bobSecret = bob.computeSecret(aliceKey);

assert.strictEqual(aliceSecret.toString('hex'), bobSecret.toString('hex'));
// OK
```

### Static method: `ECDH.convertKey(key, curve[, inputEncoding[, outputEncoding[, format]]])`

<!-- YAML
added: v10.0.0
-->

* `key` {string|ArrayBuffer|Buffer|TypedArray|DataView}
* `curve` {string}
* `inputEncoding` {string} The [encoding][] of the `key` string.
* `outputEncoding` {string} The [encoding][] of the return value.
* `format` {string} **Default:** `'uncompressed'`
* Returns: {Buffer | string}

Converts the EC Diffie-Hellman public key specified by `key` and `curve` to the
format specified by `format`. The `format` argument specifies point encoding
and can be `'compressed'`, `'uncompressed'` or `'hybrid'`. The supplied key is
interpreted using the specified `inputEncoding`, and the returned key is encoded
using the specified `outputEncoding`.

Use [`crypto.getCurves()`][] to obtain a list of available curve names.
On recent OpenSSL releases, `openssl ecparam -list_curves` will also display
the name and description of each available elliptic curve.

If `format` is not specified the point will be returned in `'uncompressed'`
format.

If the `inputEncoding` is not provided, `key` is expected to be a [`Buffer`][],
`TypedArray`, or `DataView`.

Example (uncompressing a key):

```mjs
const {
  createECDH,
  ECDH,
} = await import('node:crypto');

const ecdh = createECDH('secp256k1');
ecdh.generateKeys();

const compressedKey = ecdh.getPublicKey('hex', 'compressed');

const uncompressedKey = ECDH.convertKey(compressedKey,
                                        'secp256k1',
                                        'hex',
                                        'hex',
                                        'uncompressed');

// The converted key and the uncompressed public key should be the same
console.log(uncompressedKey === ecdh.getPublicKey('hex'));
```

```cjs
const {
  createECDH,
  ECDH,
} = require('node:crypto');

const ecdh = createECDH('secp256k1');
ecdh.generateKeys();

const compressedKey = ecdh.getPublicKey('hex', 'compressed');

const uncompressedKey = ECDH.convertKey(compressedKey,
                                        'secp256k1',
                                        'hex',
                                        'hex',
                                        'uncompressed');

// The converted key and the uncompressed public key should be the same
console.log(uncompressedKey === ecdh.getPublicKey('hex'));
```

### `ecdh.computeSecret(otherPublicKey[, inputEncoding][, outputEncoding])`

<!-- YAML
added: v0.11.14
changes:
  - version: v10.0.0
    pr-url: https://github.com/nodejs/node/pull/16849
    description: Changed error format to better support invalid public key
                 error.
  - version: v6.0.0
    pr-url: https://github.com/nodejs/node/pull/5522
    description: The default `inputEncoding` changed from `binary` to `utf8`.
-->

* `otherPublicKey` {string|ArrayBuffer|Buffer|TypedArray|DataView}
* `inputEncoding` {string} The [encoding][] of the `otherPublicKey` string.
* `outputEncoding` {string} The [encoding][] of the return value.
* Returns: {Buffer | string}

Computes the shared secret using `otherPublicKey` as the other
party's public key and returns the computed shared secret. The supplied
key is interpreted using specified `inputEncoding`, and the returned secret
is encoded using the specified `outputEncoding`.
If the `inputEncoding` is not
provided, `otherPublicKey` is expected to be a [`Buffer`][], `TypedArray`, or
`DataView`.

If `outputEncoding` is given a string will be returned; otherwise a
[`Buffer`][] is returned.

`ecdh.computeSecret` will throw an
`ERR_CRYPTO_ECDH_INVALID_PUBLIC_KEY` error when `otherPublicKey`
lies outside of the elliptic curve. Since `otherPublicKey` is
usually supplied from a remote user over an insecure network,
be sure to handle this exception accordingly.

### `ecdh.generateKeys([encoding[, format]])`

<!-- YAML
added: v0.11.14
-->

* `encoding` {string} The [encoding][] of the return value.
* `format` {string} **Default:** `'uncompressed'`
* Returns: {Buffer | string}

Generates private and public EC Diffie-Hellman key values, and returns
the public key in the specified `format` and `encoding`. This key should be
transferred to the other party.

The `format` argument specifies point encoding and can be `'compressed'` or
`'uncompressed'`. If `format` is not specified, the point will be returned in
`'uncompressed'` format.

If `encoding` is provided a string is returned; otherwise a [`Buffer`][]
is returned.

### `ecdh.getPrivateKey([encoding])`

<!-- YAML
added: v0.11.14
-->

* `encoding` {string} The [encoding][] of the return value.
* Returns: {Buffer | string} The EC Diffie-Hellman in the specified `encoding`.

If `encoding` is specified, a string is returned; otherwise a [`Buffer`][] is
returned.

### `ecdh.getPublicKey([encoding][, format])`

<!-- YAML
added: v0.11.14
-->

* `encoding` {string} The [encoding][] of the return value.
* `format` {string} **Default:** `'uncompressed'`
* Returns: {Buffer | string} The EC Diffie-Hellman public key in the specified
  `encoding` and `format`.

The `format` argument specifies point encoding and can be `'compressed'` or
`'uncompressed'`. If `format` is not specified the point will be returned in
`'uncompressed'` format.

If `encoding` is specified, a string is returned; otherwise a [`Buffer`][] is
returned.

### `ecdh.setPrivateKey(privateKey[, encoding])`

<!-- YAML
added: v0.11.14
-->

* `privateKey` {string|ArrayBuffer|Buffer|TypedArray|DataView}
* `encoding` {string} The [encoding][] of the `privateKey` string.

Sets the EC Diffie-Hellman private key.
If `encoding` is provided, `privateKey` is expected
to be a string; otherwise `privateKey` is expected to be a [`Buffer`][],
`TypedArray`, or `DataView`.

If `privateKey` is not valid for the curve specified when the `ECDH` object was
created, an error is thrown. Upon setting the private key, the associated
public point (key) is also generated and set in the `ECDH` object.

### `ecdh.setPublicKey(publicKey[, encoding])`

<!-- YAML
added: v0.11.14
deprecated: v5.2.0
-->

> Stability: 0 - Deprecated

* `publicKey` {string|ArrayBuffer|Buffer|TypedArray|DataView}
* `encoding` {string} The [encoding][] of the `publicKey` string.

Sets the EC Diffie-Hellman public key.
If `encoding` is provided `publicKey` is expected to
be a string; otherwise a [`Buffer`][], `TypedArray`, or `DataView` is expected.

There is not normally a reason to call this method because `ECDH`
only requires a private key and the other party's public key to compute the
shared secret. Typically either [`ecdh.generateKeys()`][] or
[`ecdh.setPrivateKey()`][] will be called. The [`ecdh.setPrivateKey()`][] method
attempts to generate the public point/key associated with the private key being
set.

Example (obtaining a shared secret):

```mjs
const {
  createECDH,
  createHash,
} = await import('node:crypto');

const alice = createECDH('secp256k1');
const bob = createECDH('secp256k1');

// This is a shortcut way of specifying one of Alice's previous private
// keys. It would be unwise to use such a predictable private key in a real
// application.
alice.setPrivateKey(
  createHash('sha256').update('alice', 'utf8').digest(),
);

// Bob uses a newly generated cryptographically strong
// pseudorandom key pair
bob.generateKeys();

const aliceSecret = alice.computeSecret(bob.getPublicKey(), null, 'hex');
const bobSecret = bob.computeSecret(alice.getPublicKey(), null, 'hex');

// aliceSecret and bobSecret should be the same shared secret value
console.log(aliceSecret === bobSecret);
```

```cjs
const {
  createECDH,
  createHash,
} = require('node:crypto');

const alice = createECDH('secp256k1');
const bob = createECDH('secp256k1');

// This is a shortcut way of specifying one of Alice's previous private
// keys. It would be unwise to use such a predictable private key in a real
// application.
alice.setPrivateKey(
  createHash('sha256').update('alice', 'utf8').digest(),
);

// Bob uses a newly generated cryptographically strong
// pseudorandom key pair
bob.generateKeys();

const aliceSecret = alice.computeSecret(bob.getPublicKey(), null, 'hex');
const bobSecret = bob.computeSecret(alice.getPublicKey(), null, 'hex');

// aliceSecret and bobSecret should be the same shared secret value
console.log(aliceSecret === bobSecret);
```

## Class: `Hash`

<!-- YAML
added: v0.1.92
-->

* Extends: {stream.Transform}

The `Hash` class is a utility for creating hash digests of data. It can be
used in one of two ways:

* As a [stream][] that is both readable and writable, where data is written
  to produce a computed hash digest on the readable side, or
* Using the [`hash.update()`][] and [`hash.digest()`][] methods to produce the
  computed hash.

The [`crypto.createHash()`][] method is used to create `Hash` instances. `Hash`
objects are not to be created directly using the `new` keyword.

Example: Using `Hash` objects as streams:

```mjs
const {
  createHash,
} = await import('node:crypto');

const hash = createHash('sha256');

hash.on('readable', () => {
  // Only one element is going to be produced by the
  // hash stream.
  const data = hash.read();
  if (data) {
    console.log(data.toString('hex'));
    // Prints:
    //   6a2da20943931e9834fc12cfe5bb47bbd9ae43489a30726962b576f4e3993e50
  }
});

hash.write('some data to hash');
hash.end();
```

```cjs
const {
  createHash,
} = require('node:crypto');

const hash = createHash('sha256');

hash.on('readable', () => {
  // Only one element is going to be produced by the
  // hash stream.
  const data = hash.read();
  if (data) {
    console.log(data.toString('hex'));
    // Prints:
    //   6a2da20943931e9834fc12cfe5bb47bbd9ae43489a30726962b576f4e3993e50
  }
});

hash.write('some data to hash');
hash.end();
```

Example: Using `Hash` and piped streams:

```mjs
import { createReadStream } from 'node:fs';
import { stdout } from 'node:process';
const { createHash } = await import('node:crypto');

const hash = createHash('sha256');

const input = createReadStream('test.js');
input.pipe(hash).setEncoding('hex').pipe(stdout);
```

```cjs
const { createReadStream } = require('node:fs');
const { createHash } = require('node:crypto');
const { stdout } = require('node:process');

const hash = createHash('sha256');

const input = createReadStream('test.js');
input.pipe(hash).setEncoding('hex').pipe(stdout);
```

Example: Using the [`hash.update()`][] and [`hash.digest()`][] methods:

```mjs
const {
  createHash,
} = await import('node:crypto');

const hash = createHash('sha256');

hash.update('some data to hash');
console.log(hash.digest('hex'));
// Prints:
//   6a2da20943931e9834fc12cfe5bb47bbd9ae43489a30726962b576f4e3993e50
```

```cjs
const {
  createHash,
} = require('node:crypto');

const hash = createHash('sha256');

hash.update('some data to hash');
console.log(hash.digest('hex'));
// Prints:
//   6a2da20943931e9834fc12cfe5bb47bbd9ae43489a30726962b576f4e3993e50
```

### `hash.copy([options])`

<!-- YAML
added: v13.1.0
-->

* `options` {Object} [`stream.transform` options][]
* Returns: {Hash}

Creates a new `Hash` object that contains a deep copy of the internal state
of the current `Hash` object.

The optional `options` argument controls stream behavior. For XOF hash
functions such as `'shake256'`, the `outputLength` option can be used to
specify the desired output length in bytes.

An error is thrown when an attempt is made to copy the `Hash` object after
its [`hash.digest()`][] method has been called.

```mjs
// Calculate a rolling hash.
const {
  createHash,
} = await import('node:crypto');

const hash = createHash('sha256');

hash.update('one');
console.log(hash.copy().digest('hex'));

hash.update('two');
console.log(hash.copy().digest('hex'));

hash.update('three');
console.log(hash.copy().digest('hex'));

// Etc.
```

```cjs
// Calculate a rolling hash.
const {
  createHash,
} = require('node:crypto');

const hash = createHash('sha256');

hash.update('one');
console.log(hash.copy().digest('hex'));

hash.update('two');
console.log(hash.copy().digest('hex'));

hash.update('three');
console.log(hash.copy().digest('hex'));

// Etc.
```

### `hash.digest([encoding])`

<!-- YAML
added: v0.1.92
-->

* `encoding` {string} The [encoding][] of the return value.
* Returns: {Buffer | string}

Calculates the digest of all of the data passed to be hashed (using the
[`hash.update()`][] method).
If `encoding` is provided a string will be returned; otherwise
a [`Buffer`][] is returned.

The `Hash` object can not be used again after `hash.digest()` method has been
called. Multiple calls will cause an error to be thrown.

### `hash.update(data[, inputEncoding])`

<!-- YAML
added: v0.1.92
changes:
  - version: v6.0.0
    pr-url: https://github.com/nodejs/node/pull/5522
    description: The default `inputEncoding` changed from `binary` to `utf8`.
-->

* `data` {string|Buffer|TypedArray|DataView}
* `inputEncoding` {string} The [encoding][] of the `data` string.

Updates the hash content with the given `data`, the encoding of which
is given in `inputEncoding`.
If `encoding` is not provided, and the `data` is a string, an
encoding of `'utf8'` is enforced. If `data` is a [`Buffer`][], `TypedArray`, or
`DataView`, then `inputEncoding` is ignored.

This can be called many times with new data as it is streamed.

## Class: `Hmac`

<!-- YAML
added: v0.1.94
-->

* Extends: {stream.Transform}

The `Hmac` class is a utility for creating cryptographic HMAC digests. It can
be used in one of two ways:

* As a [stream][] that is both readable and writable, where data is written
  to produce a computed HMAC digest on the readable side, or
* Using the [`hmac.update()`][] and [`hmac.digest()`][] methods to produce the
  computed HMAC digest.

The [`crypto.createHmac()`][] method is used to create `Hmac` instances. `Hmac`
objects are not to be created directly using the `new` keyword.

Example: Using `Hmac` objects as streams:

```mjs
const {
  createHmac,
} = await import('node:crypto');

const hmac = createHmac('sha256', 'a secret');

hmac.on('readable', () => {
  // Only one element is going to be produced by the
  // hash stream.
  const data = hmac.read();
  if (data) {
    console.log(data.toString('hex'));
    // Prints:
    //   7fd04df92f636fd450bc841c9418e5825c17f33ad9c87c518115a45971f7f77e
  }
});

hmac.write('some data to hash');
hmac.end();
```

```cjs
const {
  createHmac,
} = require('node:crypto');

const hmac = createHmac('sha256', 'a secret');

hmac.on('readable', () => {
  // Only one element is going to be produced by the
  // hash stream.
  const data = hmac.read();
  if (data) {
    console.log(data.toString('hex'));
    // Prints:
    //   7fd04df92f636fd450bc841c9418e5825c17f33ad9c87c518115a45971f7f77e
  }
});

hmac.write('some data to hash');
hmac.end();
```

Example: Using `Hmac` and piped streams:

```mjs
import { createReadStream } from 'node:fs';
import { stdout } from 'node:process';
const {
  createHmac,
} = await import('node:crypto');

const hmac = createHmac('sha256', 'a secret');

const input = createReadStream('test.js');
input.pipe(hmac).pipe(stdout);
```

```cjs
const {
  createReadStream,
} = require('node:fs');
const {
  createHmac,
} = require('node:crypto');
const { stdout } = require('node:process');

const hmac = createHmac('sha256', 'a secret');

const input = createReadStream('test.js');
input.pipe(hmac).pipe(stdout);
```

Example: Using the [`hmac.update()`][] and [`hmac.digest()`][] methods:

```mjs
const {
  createHmac,
} = await import('node:crypto');

const hmac = createHmac('sha256', 'a secret');

hmac.update('some data to hash');
console.log(hmac.digest('hex'));
// Prints:
//   7fd04df92f636fd450bc841c9418e5825c17f33ad9c87c518115a45971f7f77e
```

```cjs
const {
  createHmac,
} = require('node:crypto');

const hmac = createHmac('sha256', 'a secret');

hmac.update('some data to hash');
console.log(hmac.digest('hex'));
// Prints:
//   7fd04df92f636fd450bc841c9418e5825c17f33ad9c87c518115a45971f7f77e
```

### `hmac.digest([encoding])`

<!-- YAML
added: v0.1.94
-->

* `encoding` {string} The [encoding][] of the return value.
* Returns: {Buffer | string}

Calculates the HMAC digest of all of the data passed using [`hmac.update()`][].
If `encoding` is
provided a string is returned; otherwise a [`Buffer`][] is returned;

The `Hmac` object can not be used again after `hmac.digest()` has been
called. Multiple calls to `hmac.digest()` will result in an error being thrown.

### `hmac.update(data[, inputEncoding])`

<!-- YAML
added: v0.1.94
changes:
  - version: v6.0.0
    pr-url: https://github.com/nodejs/node/pull/5522
    description: The default `inputEncoding` changed from `binary` to `utf8`.
-->

* `data` {string|Buffer|TypedArray|DataView}
* `inputEncoding` {string} The [encoding][] of the `data` string.

Updates the `Hmac` content with the given `data`, the encoding of which
is given in `inputEncoding`.
If `encoding` is not provided, and the `data` is a string, an
encoding of `'utf8'` is enforced. If `data` is a [`Buffer`][], `TypedArray`, or
`DataView`, then `inputEncoding` is ignored.

This can be called many times with new data as it is streamed.

## Class: `KeyObject`

<!-- YAML
added: v11.6.0
changes:
  - version:
    - v14.5.0
    - v12.19.0
    pr-url: https://github.com/nodejs/node/pull/33360
    description: Instances of this class can now be passed to worker threads
                 using `postMessage`.
  - version: v11.13.0
    pr-url: https://github.com/nodejs/node/pull/26438
    description: This class is now exported.
-->

Node.js uses a `KeyObject` class to represent a symmetric or asymmetric key,
and each kind of key exposes different functions. The
[`crypto.createSecretKey()`][], [`crypto.createPublicKey()`][] and
[`crypto.createPrivateKey()`][] methods are used to create `KeyObject`
instances. `KeyObject` objects are not to be created directly using the `new`
keyword.

Most applications should consider using the new `KeyObject` API instead of
passing keys as strings or `Buffer`s due to improved security features.

`KeyObject` instances can be passed to other threads via [`postMessage()`][].
The receiver obtains a cloned `KeyObject`, and the `KeyObject` does not need to
be listed in the `transferList` argument.

### Static method: `KeyObject.from(key)`

<!-- YAML
added: v15.0.0
-->

* `key` {CryptoKey}
const {
  createDiffieHellman,
} = require('node:crypto');

// Generate Alice's keys...
const alice = createDiffieHellman(2048);
const aliceKey = alice.generateKeys();

// Generate Bob's keys...
const bob = createDiffieHellman(alice.getPrime(), alice.getGenerator());
const bobKey = bob.generateKeys();

// Exchange and generate the secret...
const aliceSecret = alice.computeSecret(bobKey);
const bobSecret = bob.computeSecret(aliceKey);

// OK
assert.strictEqual(aliceSecret.toString('hex'), bobSecret.toString('hex'));
```

### `diffieHellman.computeSecret(otherPublicKey[, inputEncoding][, outputEncoding])`

<!-- YAML
added: v0.5.0
-->

* `otherPublicKey` {string|ArrayBuffer|Buffer|TypedArray|DataView}
* `inputEncoding` {string} The [encoding][] of an `otherPublicKey` string.
* `outputEncoding` {string} The [encoding][] of the return value.
* Returns: {Buffer | string}

Computes the shared secret using `otherPublicKey` as the other
party's public key and returns the computed shared secret. The supplied
key is interpreted using the specified `inputEncoding`, and secret is
encoded using specified `outputEncoding`.
If the `inputEncoding` is not
provided, `otherPublicKey` is expected to be a [`Buffer`][],
`TypedArray`, or `DataView`.

If `outputEncoding` is given a string is returned; otherwise, a
[`Buffer`][] is returned.

### `diffieHellman.generateKeys([encoding])`

<!-- YAML
added: v0.5.0
-->

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
-->

* `encoding` {string} The [encoding][] of the return value.
* Returns: {Buffer | string}

Returns the Diffie-Hellman generator in the specified `encoding`.
If `encoding` is provided a string is
returned; otherwise a [`Buffer`][] is returned.

### `diffieHellman.getPrime([encoding])`

<!-- YAML
added: v0.5.0
-->

* `encoding` {string} The [encoding][] of the return value.
* Returns: {Buffer | string}

Returns the Diffie-Hellman prime in the specified `encoding`.
If `encoding` is provided a string is
returned; otherwise a [`Buffer`][] is returned.

### `diffieHellman.getPrivateKey([encoding])`

<!-- YAML
added: v0.5.0
-->

* `encoding` {string} The [encoding][] of the return value.
* Returns: {Buffer | string}

Returns the Diffie-Hellman private key in the specified `encoding`.
If `encoding` is provided a
string is returned; otherwise a [`Buffer`][] is returned.

### `diffieHellman.getPublicKey([encoding])`

<!-- YAML
added: v0.5.0
-->

* `encoding` {string} The [encoding][] of the return value.
* Returns: {Buffer | string}

Returns the Diffie-Hellman public key in the specified `encoding`.
If `encoding` is provided a
string is returned; otherwise a [`Buffer`][] is returned.

### `diffieHellman.setPrivateKey(privateKey[, encoding])`

<!-- YAML
added: v0.5.0
-->

* `privateKey` {string|ArrayBuffer|Buffer|TypedArray|DataView}
* `encoding` {string} The [encoding][] of the `privateKey` string.

Sets the Diffie-Hellman private key. If the `encoding` argument is provided,
`privateKey` is expected
to be a string. If no `encoding` is provided, `privateKey` is expected
to be a [`Buffer`][], `TypedArray`, or `DataView`.

### `diffieHellman.setPublicKey(publicKey[, encoding])`

<!-- YAML
added: v0.5.0
-->

* `publicKey` {string|ArrayBuffer|Buffer|TypedArray|DataView}
* `encoding` {string} The [encoding][] of the `publicKey` string.

Sets the Diffie-Hellman public key. If the `encoding` argument is provided,
`publicKey` is expected
to be a string. If no `encoding` is provided, `publicKey` is expected
to be a [`Buffer`][], `TypedArray`, or `DataView`.

### `diffieHellman.verifyError`

<!-- YAML
added: v0.11.12
-->

A bit field containing any warnings and/or errors resulting from a check
performed during initialization of the `DiffieHellman` object.

The following values are valid for this property (as defined in `node:constants` module):

* `DH_CHECK_P_NOT_SAFE_PRIME`
* `DH_CHECK_P_NOT_PRIME`
* `DH_UNABLE_TO_CHECK_GENERATOR`
* `DH_NOT_SUITABLE_GENERATOR`

## Class: `DiffieHellmanGroup`

<!-- YAML
added: v0.7.5
-->

The `DiffieHellmanGroup` class takes a well-known modp group as its argument.
It works the same as `DiffieHellman`, except that it does not allow changing
its keys after creation. In other words, it does not implement `setPublicKey()`
or `setPrivateKey()` methods.

```mjs
const { createDiffieHellmanGroup } = await import('node:crypto');
const dh = createDiffieHellmanGroup('modp16');
```

```cjs
const { createDiffieHellmanGroup } = require('node:crypto');
const dh = createDiffieHellmanGroup('modp16');
```

The following groups are supported:

* `'modp14'` (2048 bits, [RFC 3526][] Section 3)
* `'modp15'` (3072 bits, [RFC 3526][] Section 4)
* `'modp16'` (4096 bits, [RFC 3526][] Section 5)
* `'modp17'` (6144 bits, [RFC 3526][] Section 6)
* `'modp18'` (8192 bits, [RFC 3526][] Section 7)

The following groups are still supported but deprecated (see [Caveats][]):

* `'modp1'` (768 bits, [RFC 2409][] Section 6.1) <span class="deprecated-inline"></span>
* `'modp2'` (1024 bits, [RFC 2409][] Section 6.2) <span class="deprecated-inline"></span>
* `'modp5'` (1536 bits, [RFC 3526][] Section 2) <span class="deprecated-inline"></span>

These deprecated groups might be removed in future versions of Node.js.

## Class: `ECDH`

<!-- YAML
added: v0.11.14
-->

The `ECDH` class is a utility for creating Elliptic Curve Diffie-Hellman (ECDH)
key exchanges.

Instances of the `ECDH` class can be created using the
[`crypto.createECDH()`][] function.

```mjs
import assert from 'node:assert';

const {
  createECDH,
} = await import('node:crypto');

// Generate Alice's keys...
const alice = createECDH('secp521r1');
const aliceKey = alice.generateKeys();

// Generate Bob's keys...
const bob = createECDH('secp521r1');
const bobKey = bob.generateKeys();

// Exchange and generate the secret...
const aliceSecret = alice.computeSecret(bobKey);
const bobSecret = bob.computeSecret(aliceKey);

assert.strictEqual(aliceSecret.toString('hex'), bobSecret.toString('hex'));
// OK
```

```cjs
const assert = require('node:assert');

const {
  createECDH,
} = require('node:crypto');

// Generate Alice's keys...
const alice = createECDH('secp521r1');
const aliceKey = alice.generateKeys();

// Generate Bob's keys...
const bob = createECDH('secp521r1');
const bobKey = bob.generateKeys();

// Exchange and generate the secret...
const aliceSecret = alice.computeSecret(bobKey);
const bobSecret = bob.computeSecret(aliceKey);

assert.strictEqual(aliceSecret.toString('hex'), bobSecret.toString('hex'));
// OK
```

### Static method: `ECDH.convertKey(key, curve[, inputEncoding[, outputEncoding[, format]]])`

<!-- YAML
added: v10.0.0
-->

* `key` {string|ArrayBuffer|Buffer|TypedArray|DataView}
} catch (err) {
  throw new Error('Authentication failed!', { cause: err });
}

console.log(receivedPlaintext);
```

### FIPS mode

When using OpenSSL 3, Node.js supports FIPS 140-2 when used with an appropriate
OpenSSL 3 provider, such as the [FIPS provider from OpenSSL 3][] which can be
installed by following the instructions in [OpenSSL's FIPS README file][].

For FIPS support in Node.js you will need:

* A correctly installed OpenSSL 3 FIPS provider.
* An OpenSSL 3 [FIPS module configuration file][].
* An OpenSSL 3 configuration file that references the FIPS module
  configuration file.

Node.js will need to be configured with an OpenSSL configuration file that
points to the FIPS provider. An example configuration file looks like this:

```text
nodejs_conf = nodejs_init

.include /<absolute path>/fipsmodule.cnf

[nodejs_init]
providers = provider_sect

[provider_sect]
default = default_sect
# The fips section name should match the section name inside the
# included fipsmodule.cnf.
fips = fips_sect

[default_sect]
activate = 1
```

where `fipsmodule.cnf` is the FIPS module configuration file generated from the
FIPS provider installation step:

```bash
openssl fipsinstall
```

Set the `OPENSSL_CONF` environment variable to point to
your configuration file and `OPENSSL_MODULES` to the location of the FIPS
provider dynamic library. e.g.

```bash
export OPENSSL_CONF=/<path to configuration file>/nodejs.cnf
export OPENSSL_MODULES=/<path to openssl lib>/ossl-modules
```

FIPS mode can then be enabled in Node.js either by:

* Starting Node.js with `--enable-fips` or `--force-fips` command line flags.
* Programmatically calling `crypto.setFips(true)`.

Optionally FIPS mode can be enabled in Node.js via the OpenSSL configuration
file. e.g.

```text
nodejs_conf = nodejs_init

.include /<absolute path>/fipsmodule.cnf

[nodejs_init]
providers = provider_sect
alg_section = algorithm_sect

[provider_sect]
default = default_sect
# The fips section name should match the section name inside the
# included fipsmodule.cnf.
fips = fips_sect

[default_sect]
activate = 1

[algorithm_sect]
default_properties = fips=yes
```

## Crypto constants

The following constants exported by `crypto.constants` apply to various uses of
the `node:crypto`, `node:tls`, and `node:https` modules and are generally
specific to OpenSSL.

### OpenSSL options

See the [list of SSL OP Flags][] for details.

<table>
  <tr>
    <th>Constant</th>
    <th>Description</th>
  </tr>
  <tr>
    <td><code>SSL_OP_ALL</code></td>
    <td>Applies multiple bug workarounds within OpenSSL. See
    <a href="https://www.openssl.org/docs/man3.0/man3/SSL_CTX_set_options.html">https://www.openssl.org/docs/man3.0/man3/SSL_CTX_set_options.html</a>
    for detail.</td>
  </tr>
  <tr>
    <td><code>SSL_OP_ALLOW_NO_DHE_KEX</code></td>
    <td>Instructs OpenSSL to allow a non-[EC]DHE-based key exchange mode
    for TLS v1.3</td>
  </tr>
  <tr>
    <td><code>SSL_OP_ALLOW_UNSAFE_LEGACY_RENEGOTIATION</code></td>
    <td>Allows legacy insecure renegotiation between OpenSSL and unpatched
    clients or servers. See
    <a href="https://www.openssl.org/docs/man3.0/man3/SSL_CTX_set_options.html">https://www.openssl.org/docs/man3.0/man3/SSL_CTX_set_options.html</a>.</td>
  </tr>
  <tr>
    <td><code>SSL_OP_CIPHER_SERVER_PREFERENCE</code></td>
    <td>Attempts to use the server's preferences instead of the client's when
    selecting a cipher. Behavior depends on protocol version. See
    <a href="https://www.openssl.org/docs/man3.0/man3/SSL_CTX_set_options.html">https://www.openssl.org/docs/man3.0/man3/SSL_CTX_set_options.html</a>.</td>
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
    <td><code>SSL_OP_LEGACY_SERVER_CONNECT</code></td>
    <td>Allows initial connection to servers that do not support RI.</td>
  </tr>
  <tr>
    <td><code>SSL_OP_NO_COMPRESSION</code></td>
    <td>Instructs OpenSSL to disable support for SSL/TLS compression.</td>
  </tr>
  <tr>
    <td><code>SSL_OP_NO_ENCRYPT_THEN_MAC</code></td>
    <td>Instructs OpenSSL to disable encrypt-then-MAC.</td>
  </tr>
  <tr>
    <td><code>SSL_OP_NO_QUERY_MTU</code></td>
    <td></td>
  </tr>
  <tr>
    <td><code>SSL_OP_NO_RENEGOTIATION</code></td>
    <td>Instructs OpenSSL to disable renegotiation.</td>
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
  <tr>
    <td><code>SSL_OP_NO_TLSv1_3</code></td>
    <td>Instructs OpenSSL to turn off TLS v1.3</td>
  </tr>
  <tr>
    <td><code>SSL_OP_PRIORITIZE_CHACHA</code></td>
    <td>Instructs OpenSSL server to prioritize ChaCha20-Poly1305
    when the client does.
    This option has no effect if
    <code>SSL_OP_CIPHER_SERVER_PREFERENCE</code>
    is not enabled.</td>
  </tr>
  <tr>
    <td><code>SSL_OP_TLS_ROLLBACK_BUG</code></td>
    <td>Instructs OpenSSL to disable version rollback attack detection.</td>
  </tr>
</table>

### OpenSSL engine constants

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

### Other OpenSSL constants

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

### Node.js crypto constants

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

[AEAD algorithms]: https://en.wikipedia.org/wiki/Authenticated_encryption
[CCM mode]: #ccm-mode
[CVE-2021-44532]: https://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2021-44532
[Caveats]: #support-for-weak-or-compromised-algorithms
[Crypto constants]: #crypto-constants
[FIPS module configuration file]: https://www.openssl.org/docs/man3.0/man5/fips_config.html
[FIPS provider from OpenSSL 3]: https://www.openssl.org/docs/man3.0/man7/crypto.html#FIPS-provider
[HTML 5.2]: https://www.w3.org/TR/html52/changes.html#features-removed
[JWK]: https://tools.ietf.org/html/rfc7517
[NIST SP 800-131A]: https://nvlpubs.nist.gov/nistpubs/SpecialPublications/NIST.SP.800-131Ar1.pdf
[NIST SP 800-132]: https://nvlpubs.nist.gov/nistpubs/Legacy/SP/nistspecialpublication800-132.pdf
[NIST SP 800-38D]: https://nvlpubs.nist.gov/nistpubs/Legacy/SP/nistspecialpublication800-38d.pdf
[Nonce-Disrespecting Adversaries]: https://github.com/nonce-disrespect/nonce-disrespect
[OpenSSL's FIPS README file]: https://github.com/openssl/openssl/blob/openssl-3.0/README-FIPS.md
[OpenSSL's SPKAC implementation]: https://www.openssl.org/docs/man3.0/man1/openssl-spkac.html
[RFC 1421]: https://www.rfc-editor.org/rfc/rfc1421.txt
[RFC 2409]: https://www.rfc-editor.org/rfc/rfc2409.txt
[RFC 2818]: https://www.rfc-editor.org/rfc/rfc2818.txt
[RFC 3526]: https://www.rfc-editor.org/rfc/rfc3526.txt
[RFC 3610]: https://www.rfc-editor.org/rfc/rfc3610.txt
[RFC 4055]: https://www.rfc-editor.org/rfc/rfc4055.txt
[RFC 4122]: https://www.rfc-editor.org/rfc/rfc4122.txt
[RFC 5208]: https://www.rfc-editor.org/rfc/rfc5208.txt
[RFC 5280]: https://www.rfc-editor.org/rfc/rfc5280.txt
[Web Crypto API documentation]: webcrypto.md
[`BN_is_prime_ex`]: https://www.openssl.org/docs/man1.1.1/man3/BN_is_prime_ex.html
[`Buffer`]: buffer.md
[`DiffieHellmanGroup`]: #class-diffiehellmangroup
[`EVP_BytesToKey`]: https://www.openssl.org/docs/man3.0/man3/EVP_BytesToKey.html
[`KeyObject`]: #class-keyobject
[`Sign`]: #class-sign
[`String.prototype.normalize()`]: https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/String/normalize
[`UV_THREADPOOL_SIZE`]: cli.md#uv_threadpool_sizesize
[`Verify`]: #class-verify
[`cipher.final()`]: #cipherfinaloutputencoding
[`cipher.update()`]: #cipherupdatedata-inputencoding-outputencoding
[`crypto.createCipher()`]: #cryptocreatecipheralgorithm-password-options
[`crypto.createCipheriv()`]: #cryptocreatecipherivalgorithm-key-iv-options
[`crypto.createDecipher()`]: #cryptocreatedecipheralgorithm-password-options
[`crypto.createDecipheriv()`]: #cryptocreatedecipherivalgorithm-key-iv-options
[`crypto.createDiffieHellman()`]: #cryptocreatediffiehellmanprime-primeencoding-generator-generatorencoding
[`crypto.createECDH()`]: #cryptocreateecdhcurvename
[`crypto.createHash()`]: #cryptocreatehashalgorithm-options
[`crypto.createHmac()`]: #cryptocreatehmacalgorithm-key-options
[`crypto.createPrivateKey()`]: #cryptocreateprivatekeykey
[`crypto.createPublicKey()`]: #cryptocreatepublickeykey
[`crypto.createSecretKey()`]: #cryptocreatesecretkeykey-encoding
[`crypto.createSign()`]: #cryptocreatesignalgorithm-options
[`crypto.createVerify()`]: #cryptocreateverifyalgorithm-options
[`crypto.generateKey()`]: #cryptogeneratekeytype-options-callback
[`crypto.getCurves()`]: #cryptogetcurves
[`crypto.getDiffieHellman()`]: #cryptogetdiffiehellmangroupname
[`crypto.getHashes()`]: #cryptogethashes
[`crypto.privateDecrypt()`]: #cryptoprivatedecryptprivatekey-buffer
[`crypto.privateEncrypt()`]: #cryptoprivateencryptprivatekey-buffer
[`crypto.publicDecrypt()`]: #cryptopublicdecryptkey-buffer
[`crypto.publicEncrypt()`]: #cryptopublicencryptkey-buffer
[`crypto.randomBytes()`]: #cryptorandombytessize-callback
[`crypto.randomFill()`]: #cryptorandomfillbuffer-offset-size-callback
[`crypto.scrypt()`]: #cryptoscryptpassword-salt-keylen-options-callback
[`crypto.webcrypto.getRandomValues()`]: webcrypto.md#cryptogetrandomvaluestypedarray
[`crypto.webcrypto.subtle`]: webcrypto.md#class-subtlecrypto
[`decipher.final()`]: #decipherfinaloutputencoding
[`decipher.update()`]: #decipherupdatedata-inputencoding-outputencoding
[`diffieHellman.setPublicKey()`]: #diffiehellmansetpublickeypublickey-encoding
[`ecdh.generateKeys()`]: #ecdhgeneratekeysencoding-format
[`ecdh.setPrivateKey()`]: #ecdhsetprivatekeyprivatekey-encoding
[`hash.digest()`]: #hashdigestencoding
[`hash.update()`]: #hashupdatedata-inputencoding
[`hmac.digest()`]: #hmacdigestencoding
[`hmac.update()`]: #hmacupdatedata-inputencoding
[`import()`]: https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Operators/import
[`keyObject.export()`]: #keyobjectexportoptions
[`postMessage()`]: worker_threads.md#portpostmessagevalue-transferlist
[`sign.sign()`]: #signsignprivatekey-outputencoding
[`sign.update()`]: #signupdatedata-inputencoding
[`stream.Writable` options]: stream.md#new-streamwritableoptions
[`stream.transform` options]: stream.md#new-streamtransformoptions
[`util.promisify()`]: util.md#utilpromisifyoriginal
[`verify.update()`]: #verifyupdatedata-inputencoding
[`verify.verify()`]: #verifyverifyobject-signature-signatureencoding
[`x509.fingerprint256`]: #x509fingerprint256
[caveats when using strings as inputs to cryptographic APIs]: #using-strings-as-inputs-to-cryptographic-apis
[certificate object]: tls.md#certificate-object
[encoding]: buffer.md#buffers-and-character-encodings
[initialization vector]: https://en.wikipedia.org/wiki/Initialization_vector
[legacy provider]: cli.md#--openssl-legacy-provider
[list of SSL OP Flags]: https://wiki.openssl.org/index.php/List_of_SSL_OP_Flags#Table_of_Options
[modulo bias]: https://en.wikipedia.org/wiki/Fisher%E2%80%93Yates_shuffle#Modulo_bias
[safe integers]: https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Number/isSafeInteger
[scrypt]: https://en.wikipedia.org/wiki/Scrypt
[stream]: stream.md
[stream-writable-write]: stream.md#writablewritechunk-encoding-callback
} catch (err) {
  throw new Error('Authentication failed!', { cause: err });
}

console.log(receivedPlaintext);
```

### FIPS mode

When using OpenSSL 3, Node.js supports FIPS 140-2 when used with an appropriate
OpenSSL 3 provider, such as the [FIPS provider from OpenSSL 3][] which can be
installed by following the instructions in [OpenSSL's FIPS README file][].

For FIPS support in Node.js you will need:

* A correctly installed OpenSSL 3 FIPS provider.
* An OpenSSL 3 [FIPS module configuration file][].
* An OpenSSL 3 configuration file that references the FIPS module
  configuration file.

Node.js will need to be configured with an OpenSSL configuration file that
points to the FIPS provider. An example configuration file looks like this:

```text
nodejs_conf = nodejs_init

.include /<absolute path>/fipsmodule.cnf

[nodejs_init]
providers = provider_sect

[provider_sect]
default = default_sect
# The fips section name should match the section name inside the
# included fipsmodule.cnf.
fips = fips_sect

[default_sect]
activate = 1
```

where `fipsmodule.cnf` is the FIPS module configuration file generated from the
FIPS provider installation step:

```bash
openssl fipsinstall
```

Set the `OPENSSL_CONF` environment variable to point to
your configuration file and `OPENSSL_MODULES` to the location of the FIPS
provider dynamic library. e.g.

```bash
export OPENSSL_CONF=/<path to configuration file>/nodejs.cnf
export OPENSSL_MODULES=/<path to openssl lib>/ossl-modules
```

FIPS mode can then be enabled in Node.js either by:

* Starting Node.js with `--enable-fips` or `--force-fips` command line flags.
* Programmatically calling `crypto.setFips(true)`.

Optionally FIPS mode can be enabled in Node.js via the OpenSSL configuration
file. e.g.

```text
nodejs_conf = nodejs_init

.include /<absolute path>/fipsmodule.cnf

[nodejs_init]
providers = provider_sect
alg_section = algorithm_sect

[provider_sect]
default = default_sect
# The fips section name should match the section name inside the
# included fipsmodule.cnf.
fips = fips_sect

[default_sect]
activate = 1

[algorithm_sect]
default_properties = fips=yes
```

## Crypto constants

The following constants exported by `crypto.constants` apply to various uses of
the `node:crypto`, `node:tls`, and `node:https` modules and are generally
specific to OpenSSL.

### OpenSSL options

See the [list of SSL OP Flags][] for details.

<table>
  <tr>
    <th>Constant</th>
    <th>Description</th>
  </tr>
  <tr>
    <td><code>SSL_OP_ALL</code></td>
    <td>Applies multiple bug workarounds within OpenSSL. See
    <a href="https://www.openssl.org/docs/man3.0/man3/SSL_CTX_set_options.html">https://www.openssl.org/docs/man3.0/man3/SSL_CTX_set_options.html</a>
    for detail.</td>
  </tr>
  <tr>
    <td><code>SSL_OP_ALLOW_NO_DHE_KEX</code></td>
    <td>Instructs OpenSSL to allow a non-[EC]DHE-based key exchange mode
    for TLS v1.3</td>
  </tr>
  <tr>
    <td><code>SSL_OP_ALLOW_UNSAFE_LEGACY_RENEGOTIATION</code></td>
    <td>Allows legacy insecure renegotiation between OpenSSL and unpatched
    clients or servers. See
    <a href="https://www.openssl.org/docs/man3.0/man3/SSL_CTX_set_options.html">https://www.openssl.org/docs/man3.0/man3/SSL_CTX_set_options.html</a>.</td>
  </tr>
  <tr>
    <td><code>SSL_OP_CIPHER_SERVER_PREFERENCE</code></td>
    <td>Attempts to use the server's preferences instead of the client's when
    selecting a cipher. Behavior depends on protocol version. See
    <a href="https://www.openssl.org/docs/man3.0/man3/SSL_CTX_set_options.html">https://www.openssl.org/docs/man3.0/man3/SSL_CTX_set_options.html</a>.</td>
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
    <td><code>SSL_OP_LEGACY_SERVER_CONNECT</code></td>
    <td>Allows initial connection to servers that do not support RI.</td>
  </tr>
  <tr>
    <td><code>SSL_OP_NO_COMPRESSION</code></td>
    <td>Instructs OpenSSL to disable support for SSL/TLS compression.</td>
  </tr>
  <tr>
    <td><code>SSL_OP_NO_ENCRYPT_THEN_MAC</code></td>
    <td>Instructs OpenSSL to disable encrypt-then-MAC.</td>
  </tr>
  <tr>
    <td><code>SSL_OP_NO_QUERY_MTU</code></td>
    <td></td>
  </tr>
  <tr>
    <td><code>SSL_OP_NO_RENEGOTIATION</code></td>
    <td>Instructs OpenSSL to disable renegotiation.</td>
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
  <tr>
    <td><code>SSL_OP_NO_TLSv1_3</code></td>
    <td>Instructs OpenSSL to turn off TLS v1.3</td>
  </tr>
  <tr>
    <td><code>SSL_OP_PRIORITIZE_CHACHA</code></td>
    <td>Instructs OpenSSL server to prioritize ChaCha20-Poly1305
    when the client does.
    This option has no effect if
    <code>SSL_OP_CIPHER_SERVER_PREFERENCE</code>
    is not enabled.</td>
  </tr>
  <tr>
    <td><code>SSL_OP_TLS_ROLLBACK_BUG</code></td>
    <td>Instructs OpenSSL to disable version rollback attack detection.</td>
  </tr>
</table>

### OpenSSL engine constants

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

### Other OpenSSL constants

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

### Node.js crypto constants

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

[AEAD algorithms]: https://en.wikipedia.org/wiki/Authenticated_encryption
[CCM mode]: #ccm-mode
[CVE-2021-44532]: https://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2021-44532
[Caveats]: #support-for-weak-or-compromised-algorithms
[Crypto constants]: #crypto-constants
[FIPS module configuration file]: https://www.openssl.org/docs/man3.0/man5/fips_config.html
[FIPS provider from OpenSSL 3]: https://www.openssl.org/docs/man3.0/man7/crypto.html#FIPS-provider
[HTML 5.2]: https://www.w3.org/TR/html52/changes.html#features-removed
[JWK]: https://tools.ietf.org/html/rfc7517
[NIST SP 800-131A]: https://nvlpubs.nist.gov/nistpubs/SpecialPublications/NIST.SP.800-131Ar1.pdf
[NIST SP 800-132]: https://nvlpubs.nist.gov/nistpubs/Legacy/SP/nistspecialpublication800-132.pdf
[NIST SP 800-38D]: https://nvlpubs.nist.gov/nistpubs/Legacy/SP/nistspecialpublication800-38d.pdf
[Nonce-Disrespecting Adversaries]: https://github.com/nonce-disrespect/nonce-disrespect
[OpenSSL's FIPS README file]: https://github.com/openssl/openssl/blob/openssl-3.0/README-FIPS.md
[OpenSSL's SPKAC implementation]: https://www.openssl.org/docs/man3.0/man1/openssl-spkac.html
[RFC 1421]: https://www.rfc-editor.org/rfc/rfc1421.txt
[RFC 2409]: https://www.rfc-editor.org/rfc/rfc2409.txt
[RFC 2818]: https://www.rfc-editor.org/rfc/rfc2818.txt
[RFC 3526]: https://www.rfc-editor.org/rfc/rfc3526.txt
[RFC 3610]: https://www.rfc-editor.org/rfc/rfc3610.txt
[RFC 4055]: https://www.rfc-editor.org/rfc/rfc4055.txt
[RFC 4122]: https://www.rfc-editor.org/rfc/rfc4122.txt
[RFC 5208]: https://www.rfc-editor.org/rfc/rfc5208.txt
[RFC 5280]: https://www.rfc-editor.org/rfc/rfc5280.txt
[Web Crypto API documentation]: webcrypto.md
[`BN_is_prime_ex`]: https://www.openssl.org/docs/man1.1.1/man3/BN_is_prime_ex.html
[`Buffer`]: buffer.md
[`DiffieHellmanGroup`]: #class-diffiehellmangroup
[`EVP_BytesToKey`]: https://www.openssl.org/docs/man3.0/man3/EVP_BytesToKey.html
[`KeyObject`]: #class-keyobject
[`Sign`]: #class-sign
[`String.prototype.normalize()`]: https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/String/normalize
[`UV_THREADPOOL_SIZE`]: cli.md#uv_threadpool_sizesize
[`Verify`]: #class-verify
[`cipher.final()`]: #cipherfinaloutputencoding
[`cipher.update()`]: #cipherupdatedata-inputencoding-outputencoding
[`crypto.createCipher()`]: #cryptocreatecipheralgorithm-password-options
[`crypto.createCipheriv()`]: #cryptocreatecipherivalgorithm-key-iv-options
[`crypto.createDecipher()`]: #cryptocreatedecipheralgorithm-password-options
[`crypto.createDecipheriv()`]: #cryptocreatedecipherivalgorithm-key-iv-options
[`crypto.createDiffieHellman()`]: #cryptocreatediffiehellmanprime-primeencoding-generator-generatorencoding
[`crypto.createECDH()`]: #cryptocreateecdhcurvename
[`crypto.createHash()`]: #cryptocreatehashalgorithm-options
[`crypto.createHmac()`]: #cryptocreatehmacalgorithm-key-options
[`crypto.createPrivateKey()`]: #cryptocreateprivatekeykey
[`crypto.createPublicKey()`]: #cryptocreatepublickeykey
[`crypto.createSecretKey()`]: #cryptocreatesecretkeykey-encoding
[`crypto.createSign()`]: #cryptocreatesignalgorithm-options
[`crypto.createVerify()`]: #cryptocreateverifyalgorithm-options
[`crypto.generateKey()`]: #cryptogeneratekeytype-options-callback
[`crypto.getCurves()`]: #cryptogetcurves
[`crypto.getDiffieHellman()`]: #cryptogetdiffiehellmangroupname
[`crypto.getHashes()`]: #cryptogethashes
[`crypto.privateDecrypt()`]: #cryptoprivatedecryptprivatekey-buffer
[`crypto.privateEncrypt()`]: #cryptoprivateencryptprivatekey-buffer
[`crypto.publicDecrypt()`]: #cryptopublicdecryptkey-buffer
[`crypto.publicEncrypt()`]: #cryptopublicencryptkey-buffer
[`crypto.randomBytes()`]: #cryptorandombytessize-callback
[`crypto.randomFill()`]: #cryptorandomfillbuffer-offset-size-callback
[`crypto.scrypt()`]: #cryptoscryptpassword-salt-keylen-options-callback
[`crypto.webcrypto.getRandomValues()`]: webcrypto.md#cryptogetrandomvaluestypedarray
[`crypto.webcrypto.subtle`]: webcrypto.md#class-subtlecrypto
[`decipher.final()`]: #decipherfinaloutputencoding
[`decipher.update()`]: #decipherupdatedata-inputencoding-outputencoding
[`diffieHellman.setPublicKey()`]: #diffiehellmansetpublickeypublickey-encoding
[`ecdh.generateKeys()`]: #ecdhgeneratekeysencoding-format
[`ecdh.setPrivateKey()`]: #ecdhsetprivatekeyprivatekey-encoding
[`hash.digest()`]: #hashdigestencoding
[`hash.update()`]: #hashupdatedata-inputencoding
[`hmac.digest()`]: #hmacdigestencoding
[`hmac.update()`]: #hmacupdatedata-inputencoding
[`import()`]: https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Operators/import
[`keyObject.export()`]: #keyobjectexportoptions
[`postMessage()`]: worker_threads.md#portpostmessagevalue-transferlist
[`sign.sign()`]: #signsignprivatekey-outputencoding
[`sign.update()`]: #signupdatedata-inputencoding
[`stream.Writable` options]: stream.md#new-streamwritableoptions
[`stream.transform` options]: stream.md#new-streamtransformoptions
[`util.promisify()`]: util.md#utilpromisifyoriginal
[`verify.update()`]: #verifyupdatedata-inputencoding
[`verify.verify()`]: #verifyverifyobject-signature-signatureencoding
[`x509.fingerprint256`]: #x509fingerprint256
[caveats when using strings as inputs to cryptographic APIs]: #using-strings-as-inputs-to-cryptographic-apis
[certificate object]: tls.md#certificate-object
[encoding]: buffer.md#buffers-and-character-encodings
[initialization vector]: https://en.wikipedia.org/wiki/Initialization_vector
[legacy provider]: cli.md#--openssl-legacy-provider
[list of SSL OP Flags]: https://wiki.openssl.org/index.php/List_of_SSL_OP_Flags#Table_of_Options
[modulo bias]: https://en.wikipedia.org/wiki/Fisher%E2%80%93Yates_shuffle#Modulo_bias
[safe integers]: https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Number/isSafeInteger
[scrypt]: https://en.wikipedia.org/wiki/Scrypt
[stream]: stream.md
[stream-writable-write]: stream.md#writablewritechunk-encoding-callback