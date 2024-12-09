readable.on('data', (chunk) => {
  console.log(chunk);
});
```

Calling `Readable.from(string)` or `Readable.from(buffer)` will not have
the strings or buffers be iterated to match the other streams semantics
for performance reasons.

### `stream.Readable.fromWeb(readableStream[, options])`

<!-- YAML
added: v17.0.0
-->

> Stability: 1 - Experimental

* `readableStream` {ReadableStream}
* `options` {Object}
  * `encoding` {string}
  * `highWaterMark` {number}
  * `objectMode` {boolean}
  * `signal` {AbortSignal}
* Returns: {stream.Readable}

### `stream.Readable.isDisturbed(stream)`

<!-- YAML
added: v16.8.0
-->

> Stability: 1 - Experimental

* `stream` {stream.Readable|ReadableStream}
* Returns: `boolean`

Returns whether the stream has been read from or cancelled.

### `stream.isErrored(stream)`

<!-- YAML
added: v17.3.0
-->

> Stability: 1 - Experimental

* `stream` {Readable|Writable|Duplex|WritableStream|ReadableStream}
* Returns: {boolean}

Returns whether the stream has encountered an error.

### `stream.Readable.toWeb(streamReadable)`

<!-- YAML
added: v17.0.0
-->

> Stability: 1 - Experimental

* `streamReadable` {stream.Readable}
* Returns: {ReadableStream}

### `stream.Writable.fromWeb(writableStream[, options])`

<!-- YAML
added: v17.0.0
-->

> Stability: 1 - Experimental

* `writableStream` {WritableStream}
* `options` {Object}
  * `decodeStrings` {boolean}
  * `highWaterMark` {number}
  * `objectMode` {boolean}
  * `signal` {AbortSignal}
* Returns: {stream.Writable}

### `stream.Writable.toWeb(streamWritable)`

<!-- YAML
added: v17.0.0
-->

> Stability: 1 - Experimental

* `streamWritable` {stream.Writable}
* Returns: {WritableStream}

### `stream.Duplex.from(src)`

<!-- YAML
added: v16.8.0
-->

* `src` {Stream|Blob|ArrayBuffer|string|Iterable|AsyncIterable|
  AsyncGeneratorFunction|AsyncFunction|Promise|Object}

A utility method for creating duplex streams.

* `Stream` converts writable stream into writable `Duplex` and readable stream
  to `Duplex`.
* `Blob` converts into readable `Duplex`.
* `string` converts into readable `Duplex`.
* `ArrayBuffer` converts into readable `Duplex`.
* `AsyncIterable` converts into a readable `Duplex`. Cannot yield
  `null`.
* `AsyncGeneratorFunction` converts into a readable/writable transform
  `Duplex`. Must take a source `AsyncIterable` as first parameter. Cannot yield
  `null`.
* `AsyncFunction` converts into a writable `Duplex`. Must return
  either `null` or `undefined`
* `Object ({ writable, readable })` converts `readable` and
  `writable` into `Stream` and then combines them into `Duplex` where the
  `Duplex` will write to the `writable` and read from the `readable`.
* `Promise` converts into readable `Duplex`. Value `null` is ignored.
* Returns: {stream.Duplex}

### `stream.Duplex.fromWeb(pair[, options])`

<!-- YAML
added: v17.0.0
-->

> Stability: 1 - Experimental

* `pair` {Object}
  * `readable` {ReadableStream}
  * `writable` {WritableStream}
* `options` {Object}
  * `allowHalfOpen` {boolean}
  * `decodeStrings` {boolean}
  * `encoding` {string}
  * `highWaterMark` {number}
  * `objectMode` {boolean}
  * `signal` {AbortSignal}
* Returns: {stream.Duplex}

### `stream.Duplex.toWeb(streamDuplex)`

<!-- YAML
added: v17.0.0
-->

> Stability: 1 - Experimental

* `streamDuplex` {stream.Duplex}
* Returns: {Object}
  * `readable` {ReadableStream}
  * `writable` {WritableStream}

### `stream.addAbortSignal(signal, stream)`

<!-- YAML
added: v15.4.0
-->

* `signal` {AbortSignal} A signal representing possible cancellation
* `stream` {Stream} a stream to attach a signal to

Attaches an AbortSignal to a readable or writeable stream. This lets code
control stream destruction using an `AbortController`.

Calling `abort` on the `AbortController` corresponding to the passed
`AbortSignal` will behave the same way as calling `.destroy(new AbortError())`
on the stream.

```js
const fs = require('fs');

const controller = new AbortController();
const read = addAbortSignal(
  controller.signal,
  fs.createReadStream(('object.json'))
);
// Later, abort the operation closing the stream
controller.abort();
```

Or using an `AbortSignal` with a readable stream as an async iterable:

```js
const controller = new AbortController();
setTimeout(() => controller.abort(), 10_000); // set a timeout
const stream = addAbortSignal(
  controller.signal,
  fs.createReadStream(('object.json'))
);
(async () => {
  try {
    for await (const chunk of stream) {
      await process(chunk);
    }
  } catch (e) {
    if (e.name === 'AbortError') {
      // The operation was cancelled
    } else {
      throw e;
    }
  }
})();
```

## API for stream implementers

<!--type=misc-->

The `stream` module API has been designed to make it possible to easily
implement streams using JavaScript's prototypal inheritance model.

First, a stream developer would declare a new JavaScript class that extends one
of the four basic stream classes (`stream.Writable`, `stream.Readable`,
`stream.Duplex`, or `stream.Transform`), making sure they call the appropriate
parent class constructor:

<!-- eslint-disable no-useless-constructor -->

```js
const { Writable } = require('stream');

class MyWritable extends Writable {
  constructor({ highWaterMark, ...options }) {
    super({ highWaterMark });
    // ...
  }
}
```

When extending streams, keep in mind what options the user
can and should provide before forwarding these to the base constructor. For
example, if the implementation makes assumptions in regard to the
`autoDestroy` and `emitClose` options, do not allow the
user to override these. Be explicit about what
options are forwarded instead of implicitly forwarding all options.

The new stream class must then implement one or more specific methods, depending
on the type of stream being created, as detailed in the chart below:

| Use-case                                      | Class           | Method(s) to implement                                                                                             |
| --------------------------------------------- | --------------- | ------------------------------------------------------------------------------------------------------------------ |
| Reading only                                  | [`Readable`][]  | [`_read()`][stream-_read]                                                                                          |
| Writing only                                  | [`Writable`][]  | [`_write()`][stream-_write], [`_writev()`][stream-_writev], [`_final()`][stream-_final]                            |
| Reading and writing                           | [`Duplex`][]    | [`_read()`][stream-_read], [`_write()`][stream-_write], [`_writev()`][stream-_writev], [`_final()`][stream-_final] |
| Operate on written data, then read the result | [`Transform`][] | [`_transform()`][stream-_transform], [`_flush()`][stream-_flush], [`_final()`][stream-_final]                      |

The implementation code for a stream should _never_ call the "public" methods
of a stream that are intended for use by consumers (as described in the
[API for stream consumers][] section). Doing so may lead to adverse side effects
in application code consuming the stream.

Avoid overriding public methods such as `write()`, `end()`, `cork()`,
`uncork()`, `read()` and `destroy()`, or emitting internal events such
as `'error'`, `'data'`, `'end'`, `'finish'` and `'close'` through `.emit()`.
Doing so can break current and future stream invariants leading to behavior
and/or compatibility issues with other streams, stream utilities, and user
expectations.

### Simplified construction

<!-- YAML
added: v1.2.0
-->

For many simple cases, it is possible to create a stream without relying on
inheritance. This can be accomplished by directly creating instances of the
`stream.Writable`, `stream.Readable`, `stream.Duplex` or `stream.Transform`
objects and passing appropriate methods as constructor options.

```js
const { Writable } = require('stream');

const myWritable = new Writable({
  construct(callback) {
    // Initialize state and load resources...
  },
  write(chunk, encoding, callback) {
    // ...
  },
  destroy() {
    // Free resources...
  }
});
```

### Implementing a writable stream

The `stream.Writable` class is extended to implement a [`Writable`][] stream.

Custom `Writable` streams _must_ call the `new stream.Writable([options])`
constructor and implement the `writable._write()` and/or `writable._writev()`
method.

#### `new stream.Writable([options])`

<!-- YAML
changes:
  - version: v15.5.0
    pr-url: https://github.com/nodejs/node/pull/36431
    description: support passing in an AbortSignal.
  - version: v14.0.0
    pr-url: https://github.com/nodejs/node/pull/30623
    description: Change `autoDestroy` option default to `true`.
  - version:
     - v11.2.0
     - v10.16.0
    pr-url: https://github.com/nodejs/node/pull/22795
    description: Add `autoDestroy` option to automatically `destroy()` the
                 stream when it emits `'finish'` or errors.
  - version: v10.0.0
    pr-url: https://github.com/nodejs/node/pull/18438
    description: Add `emitClose` option to specify if `'close'` is emitted on
                 destroy.
-->

* `options` {Object}
  * `highWaterMark` {number} Buffer level when
    [`stream.write()`][stream-write] starts returning `false`. **Default:**
    `16384` (16 KB), or `16` for `objectMode` streams.
  * `decodeStrings` {boolean} Whether to encode `string`s passed to
    [`stream.write()`][stream-write] to `Buffer`s (with the encoding
    specified in the [`stream.write()`][stream-write] call) before passing
    them to [`stream._write()`][stream-_write]. Other types of data are not
    converted (i.e. `Buffer`s are not decoded into `string`s). Setting to
    false will prevent `string`s from being converted. **Default:** `true`.
  * `defaultEncoding` {string} The default encoding that is used when no
    encoding is specified as an argument to [`stream.write()`][stream-write].
    **Default:** `'utf8'`.
  * `objectMode` {boolean} Whether or not the
    [`stream.write(anyObj)`][stream-write] is a valid operation. When set,
    it becomes possible to write JavaScript values other than string,
    `Buffer` or `Uint8Array` if supported by the stream implementation.
    **Default:** `false`.
  * `emitClose` {boolean} Whether or not the stream should emit `'close'`
    after it has been destroyed. **Default:** `true`.
  * `write` {Function} Implementation for the
    [`stream._write()`][stream-_write] method.
  * `writev` {Function} Implementation for the
    [`stream._writev()`][stream-_writev] method.
  * `destroy` {Function} Implementation for the
    [`stream._destroy()`][writable-_destroy] method.
  * `final` {Function} Implementation for the
    [`stream._final()`][stream-_final] method.
  * `construct` {Function} Implementation for the
    [`stream._construct()`][writable-_construct] method.
  * `autoDestroy` {boolean} Whether this stream should automatically call
    `.destroy()` on itself after ending. **Default:** `true`.
  * `signal` {AbortSignal} A signal representing possible cancellation.

<!-- eslint-disable no-useless-constructor -->

```js
const { Writable } = require('stream');

class MyWritable extends Writable {
  constructor(options) {
    // Calls the stream.Writable() constructor.
    super(options);
    // ...
  }
}
```

Or, when using pre-ES6 style constructors:

```js
const { Writable } = require('stream');
const util = require('util');

function MyWritable(options) {
  if (!(this instanceof MyWritable))
    return new MyWritable(options);
  Writable.call(this, options);
}
util.inherits(MyWritable, Writable);
```

Or, using the simplified constructor approach:

```js
const { Writable } = require('stream');

const myWritable = new Writable({
  write(chunk, encoding, callback) {
    // ...
  },
  writev(chunks, callback) {
    // ...
  }
});
```

Calling `abort` on the `AbortController` corresponding to the passed
`AbortSignal` will behave the same way as calling `.destroy(new AbortError())`
on the writeable stream.

```js
const { Writable } = require('stream');

const controller = new AbortController();
const myWritable = new Writable({
  write(chunk, encoding, callback) {
    // ...
  },
  writev(chunks, callback) {
    // ...
  },
  signal: controller.signal
});
// Later, abort the operation closing the stream
controller.abort();
```

#### `writable._construct(callback)`

<!-- YAML
added: v15.0.0
-->

* `callback` {Function} Call this function (optionally with an error
  argument) when the stream has finished initializing.

The `_construct()` method MUST NOT be called directly. It may be implemented
by child classes, and if so, will be called by the internal `Writable`
class methods only.

This optional function will be called in a tick after the stream constructor
has returned, delaying any `_write()`, `_final()` and `_destroy()` calls until
`callback` is called. This is useful to initialize state or asynchronously
initialize resources before the stream can be used.

```js
const { Writable } = require('stream');
const fs = require('fs');

class WriteStream extends Writable {
  constructor(filename) {
    super();
    this.filename = filename;
    this.fd = null;
  }
  _construct(callback) {
    fs.open(this.filename, (err, fd) => {
      if (err) {
        callback(err);
      } else {
        this.fd = fd;
        callback();
      }
    });
  }
  _write(chunk, encoding, callback) {
    fs.write(this.fd, chunk, callback);
  }
  _destroy(err, callback) {
    if (this.fd) {
      fs.close(this.fd, (er) => callback(er || err));
    } else {
      callback(err);
    }
  }
}
```

#### `writable._write(chunk, encoding, callback)`

<!-- YAML
changes:
  - version: v12.11.0
    pr-url: https://github.com/nodejs/node/pull/29639
    description: _write() is optional when providing _writev().
-->

* `chunk` {Buffer|string|any} The `Buffer` to be written, converted from the
  `string` passed to [`stream.write()`][stream-write]. If the stream's
  `decodeStrings` option is `false` or the stream is operating in object mode,
  the chunk will not be converted & will be whatever was passed to
  [`stream.write()`][stream-write].
* `encoding` {string} If the chunk is a string, then `encoding` is the
  character encoding of that string. If chunk is a `Buffer`, or if the
  stream is operating in object mode, `encoding` may be ignored.
* `callback` {Function} Call this function (optionally with an error
  argument) when processing is complete for the supplied chunk.

All `Writable` stream implementations must provide a
[`writable._write()`][stream-_write] and/or
[`writable._writev()`][stream-_writev] method to send data to the underlying
resource.

[`Transform`][] streams provide their own implementation of the
[`writable._write()`][stream-_write].

This function MUST NOT be called by application code directly. It should be
implemented by child classes, and called by the internal `Writable` class
methods only.

The `callback` function must be called synchronously inside of
`writable._write()` or asynchronously (i.e. different tick) to signal either
that the write completed successfully or failed with an error.
The first argument passed to the `callback` must be the `Error` object if the
call failed or `null` if the write succeeded.

All calls to `writable.write()` that occur between the time `writable._write()`
is called and the `callback` is called will cause the written data to be
buffered. When the `callback` is invoked, the stream might emit a [`'drain'`][]
event. If a stream implementation is capable of processing multiple chunks of
data at once, the `writable._writev()` method should be implemented.

If the `decodeStrings` property is explicitly set to `false` in the constructor
options, then `chunk` will remain the same object that is passed to `.write()`,
and may be a string rather than a `Buffer`. This is to support implementations
that have an optimized handling for certain string data encodings. In that case,
the `encoding` argument will indicate the character encoding of the string.
Otherwise, the `encoding` argument can be safely ignored.

The `writable._write()` method is prefixed with an underscore because it is
internal to the class that defines it, and should never be called directly by
user programs.

#### `writable._writev(chunks, callback)`

* `chunks` {Object\[]} The data to be written. The value is an array of {Object}
  that each represent a discrete chunk of data to write. The properties of
  these objects are:
  * `chunk` {Buffer|string} A buffer instance or string containing the data to
    be written. The `chunk` will be a string if the `Writable` was created with
    the `decodeStrings` option set to `false` and a string was passed to `write()`.
  * `encoding` {string} The character encoding of the `chunk`. If `chunk` is
    a `Buffer`, the `encoding` will be `'buffer'`.
* `callback` {Function} A callback function (optionally with an error
  argument) to be invoked when processing is complete for the supplied chunks.

This function MUST NOT be called by application code directly. It should be
implemented by child classes, and called by the internal `Writable` class
methods only.

The `writable._writev()` method may be implemented in addition or alternatively
to `writable._write()` in stream implementations that are capable of processing
multiple chunks of data at once. If implemented and if there is buffered data
from previous writes, `_writev()` will be called instead of `_write()`.

The `writable._writev()` method is prefixed with an underscore because it is
internal to the class that defines it, and should never be called directly by
user programs.

#### `writable._destroy(err, callback)`

<!-- YAML
added: v8.0.0
-->

* `err` {Error} A possible error.
* `callback` {Function} A callback function that takes an optional error
  argument.

The `_destroy()` method is called by [`writable.destroy()`][writable-destroy].
It can be overridden by child classes but it **must not** be called directly.

#### `writable._final(callback)`

<!-- YAML
added: v8.0.0
-->

* `callback` {Function} Call this function (optionally with an error
  argument) when finished writing any remaining data.

The `_final()` method **must not** be called directly. It may be implemented
by child classes, and if so, will be called by the internal `Writable`
class methods only.

This optional function will be called before the stream closes, delaying the
`'finish'` event until `callback` is called. This is useful to close resources
or write buffered data before a stream ends.

#### Errors while writing

Errors occurring during the processing of the [`writable._write()`][],
[`writable._writev()`][] and [`writable._final()`][] methods must be propagated
by invoking the callback and passing the error as the first argument.
Throwing an `Error` from within these methods or manually emitting an `'error'`
event results in undefined behavior.

If a `Readable` stream pipes into a `Writable` stream when `Writable` emits an
error, the `Readable` stream will be unpiped.

```js
const { Writable } = require('stream');

const myWritable = new Writable({
  write(chunk, encoding, callback) {
    if (chunk.toString().indexOf('a') >= 0) {
      callback(new Error('chunk is invalid'));
    } else {
      callback();
    }
  }
});
```

#### An example writable stream

The following illustrates a rather simplistic (and somewhat pointless) custom
`Writable` stream implementation. While this specific `Writable` stream instance
is not of any real particular usefulness, the example illustrates each of the
required elements of a custom [`Writable`][] stream instance:

```js
const { Writable } = require('stream');

class MyWritable extends Writable {
  _write(chunk, encoding, callback) {
    if (chunk.toString().indexOf('a') >= 0) {
      callback(new Error('chunk is invalid'));
    } else {
      callback();
    }
  }
}
```

#### Decoding buffers in a writable stream

Decoding buffers is a common task, for instance, when using transformers whose
input is a string. This is not a trivial process when using multi-byte
characters encoding, such as UTF-8. The following example shows how to decode
multi-byte strings using `StringDecoder` and [`Writable`][].

```js
const { Writable } = require('stream');
const { StringDecoder } = require('string_decoder');

class StringWritable extends Writable {
  constructor(options) {
    super(options);
    this._decoder = new StringDecoder(options && options.defaultEncoding);
    this.data = '';
  }
  _write(chunk, encoding, callback) {
    if (encoding === 'buffer') {
      chunk = this._decoder.write(chunk);
    }
    this.data += chunk;
    callback();
  }
  _final(callback) {
    this.data += this._decoder.end();
    callback();
  }
}

const euro = [[0xE2, 0x82], [0xAC]].map(Buffer.from);
const w = new StringWritable();

w.write('currency: ');
w.write(euro[0]);
w.end(euro[1]);

console.log(w.data); // currency: €
```

### Implementing a readable stream

The `stream.Readable` class is extended to implement a [`Readable`][] stream.

Custom `Readable` streams _must_ call the `new stream.Readable([options])`
constructor and implement the [`readable._read()`][] method.

#### `new stream.Readable([options])`

<!-- YAML
changes:
  - version: v15.5.0
    pr-url: https://github.com/nodejs/node/pull/36431
    description: support passing in an AbortSignal.
  - version: v14.0.0
    pr-url: https://github.com/nodejs/node/pull/30623
    description: Change `autoDestroy` option default to `true`.
  - version:
     - v11.2.0
     - v10.16.0
    pr-url: https://github.com/nodejs/node/pull/22795
    description: Add `autoDestroy` option to automatically `destroy()` the
                 stream when it emits `'end'` or errors.
-->

* `options` {Object}
  * `highWaterMark` {number} The maximum [number of bytes][hwm-gotcha] to store
    in the internal buffer before ceasing to read from the underlying resource.
    **Default:** `16384` (16 KB), or `16` for `objectMode` streams.
  * `encoding` {string} If specified, then buffers will be decoded to
    strings using the specified encoding. **Default:** `null`.
  * `objectMode` {boolean} Whether this stream should behave
    as a stream of objects. Meaning that [`stream.read(n)`][stream-read] returns
    a single value instead of a `Buffer` of size `n`. **Default:** `false`.
  * `emitClose` {boolean} Whether or not the stream should emit `'close'`
    after it has been destroyed. **Default:** `true`.
  * `read` {Function} Implementation for the [`stream._read()`][stream-_read]
    method.
  * `destroy` {Function} Implementation for the
    [`stream._destroy()`][readable-_destroy] method.
  * `construct` {Function} Implementation for the
    [`stream._construct()`][readable-_construct] method.
  * `autoDestroy` {boolean} Whether this stream should automatically call
    `.destroy()` on itself after ending. **Default:** `true`.
  * `signal` {AbortSignal} A signal representing possible cancellation.

<!-- eslint-disable no-useless-constructor -->

```js
const { Readable } = require('stream');

class MyReadable extends Readable {
  constructor(options) {
    // Calls the stream.Readable(options) constructor.
    super(options);
    // ...
  }
}
```

Or, when using pre-ES6 style constructors:

```js
const { Readable } = require('stream');
const util = require('util');

function MyReadable(options) {
  if (!(this instanceof MyReadable))
    return new MyReadable(options);
  Readable.call(this, options);
}
util.inherits(MyReadable, Readable);
```

Or, using the simplified constructor approach:

```js
const { Readable } = require('stream');

const myReadable = new Readable({
  read(size) {
    // ...
  }
});
```

Calling `abort` on the `AbortController` corresponding to the passed
`AbortSignal` will behave the same way as calling `.destroy(new AbortError())`
on the readable created.

```js
const { Readable } = require('stream');
const controller = new AbortController();
const read = new Readable({
  read(size) {
    // ...
  },
  signal: controller.signal
});
// Later, abort the operation closing the stream
controller.abort();
```

#### `readable._construct(callback)`

<!-- YAML
added: v15.0.0
-->

* `callback` {Function} Call this function (optionally with an error
  argument) when the stream has finished initializing.

The `_construct()` method MUST NOT be called directly. It may be implemented
by child classes, and if so, will be called by the internal `Readable`
class methods only.

This optional function will be scheduled in the next tick by the stream
constructor, delaying any `_read()` and `_destroy()` calls until `callback` is
called. This is useful to initialize state or asynchronously initialize
resources before the stream can be used.

```js
const { Readable } = require('stream');
const fs = require('fs');

class ReadStream extends Readable {
  constructor(filename) {
    super();
    this.filename = filename;
    this.fd = null;
  }
  _construct(callback) {
    fs.open(this.filename, (err, fd) => {
      if (err) {
        callback(err);
      } else {
        this.fd = fd;
        callback();
      }
    });
  }
  _read(n) {
    const buf = Buffer.alloc(n);
    fs.read(this.fd, buf, 0, n, null, (err, bytesRead) => {
      if (err) {
        this.destroy(err);
      } else {
        this.push(bytesRead > 0 ? buf.slice(0, bytesRead) : null);
      }
    });
  }
  _destroy(err, callback) {
    if (this.fd) {
      fs.close(this.fd, (er) => callback(er || err));
    } else {
      callback(err);
    }
  }
}
```

#### `readable._read(size)`

<!-- YAML
added: v0.9.4
-->

* `size` {number} Number of bytes to read asynchronously

This function MUST NOT be called by application code directly. It should be
implemented by child classes, and called by the internal `Readable` class
methods only.

All `Readable` stream implementations must provide an implementation of the
[`readable._read()`][] method to fetch data from the underlying resource.

When [`readable._read()`][] is called, if data is available from the resource,
the implementation should begin pushing that data into the read queue using the
[`this.push(dataChunk)`][stream-push] method. `_read()` will be called again
after each call to [`this.push(dataChunk)`][stream-push] once the stream is
ready to accept more data. `_read()` may continue reading from the resource and
pushing data until `readable.push()` returns `false`. Only when `_read()` is
called again after it has stopped should it resume pushing additional data into
the queue.

Once the [`readable._read()`][] method has been called, it will not be called
again until more data is pushed through the [`readable.push()`][stream-push]
method. Empty data such as empty buffers and strings will not cause
[`readable._read()`][] to be called.

The `size` argument is advisory. For implementations where a "read" is a
single operation that returns data can use the `size` argument to determine how
much data to fetch. Other implementations may ignore this argument and simply
provide data whenever it becomes available. There is no need to "wait" until
`size` bytes are available before calling [`stream.push(chunk)`][stream-push].

The [`readable._read()`][] method is prefixed with an underscore because it is
internal to the class that defines it, and should never be called directly by
user programs.

#### `readable._destroy(err, callback)`

<!-- YAML
added: v8.0.0
-->

* `err` {Error} A possible error.
* `callback` {Function} A callback function that takes an optional error
  argument.

The `_destroy()` method is called by [`readable.destroy()`][readable-destroy].
It can be overridden by child classes but it **must not** be called directly.

#### `readable.push(chunk[, encoding])`

<!-- YAML
changes:
  - version: v8.0.0
    pr-url: https://github.com/nodejs/node/pull/11608
    description: The `chunk` argument can now be a `Uint8Array` instance.
-->

* `chunk` {Buffer|Uint8Array|string|null|any} Chunk of data to push into the
  read queue. For streams not operating in object mode, `chunk` must be a
  string, `Buffer` or `Uint8Array`. For object mode streams, `chunk` may be
  any JavaScript value.
* `encoding` {string} Encoding of string chunks. Must be a valid
  `Buffer` encoding, such as `'utf8'` or `'ascii'`.
* Returns: {boolean} `true` if additional chunks of data may continue to be
  pushed; `false` otherwise.

When `chunk` is a `Buffer`, `Uint8Array` or `string`, the `chunk` of data will
be added to the internal queue for users of the stream to consume.
Passing `chunk` as `null` signals the end of the stream (EOF), after which no
more data can be written.

When the `Readable` is operating in paused mode, the data added with
`readable.push()` can be read out by calling the
[`readable.read()`][stream-read] method when the [`'readable'`][] event is
emitted.

When the `Readable` is operating in flowing mode, the data added with
`readable.push()` will be delivered by emitting a `'data'` event.

The `readable.push()` method is designed to be as flexible as possible. For
example, when wrapping a lower-level source that provides some form of
pause/resume mechanism, and a data callback, the low-level source can be wrapped
by the custom `Readable` instance:

```js
// `_source` is an object with readStop() and readStart() methods,
// and an `ondata` member that gets called when it has data, and
// an `onend` member that gets called when the data is over.

class SourceWrapper extends Readable {
  constructor(options) {
    super(options);

    this._source = getLowLevelSourceObject();

    // Every time there's data, push it into the internal buffer.
    this._source.ondata = (chunk) => {
      // If push() returns false, then stop reading from source.
      if (!this.push(chunk))
        this._source.readStop();
    };

    // When the source ends, push the EOF-signaling `null` chunk.
    this._source.onend = () => {
      this.push(null);
    };
  }
  // _read() will be called when the stream wants to pull more data in.
  // The advisory size argument is ignored in this case.
  _read(size) {
    this._source.readStart();
  }
}
```

The `readable.push()` method is used to push the content
into the internal buffer. It can be driven by the [`readable._read()`][] method.

For streams not operating in object mode, if the `chunk` parameter of
`readable.push()` is `undefined`, it will be treated as empty string or
buffer. See [`readable.push('')`][] for more information.

#### Errors while reading

Errors occurring during processing of the [`readable._read()`][] must be
propagated through the [`readable.destroy(err)`][readable-_destroy] method.
Throwing an `Error` from within [`readable._read()`][] or manually emitting an
`'error'` event results in undefined behavior.

```js
const { Readable } = require('stream');

const myReadable = new Readable({
  read(size) {
    const err = checkSomeErrorCondition();
    if (err) {
      this.destroy(err);
    } else {
      // Do some work.
    }
  }
});
```

#### An example counting stream

<!--type=example-->

The following is a basic example of a `Readable` stream that emits the numerals
from 1 to 1,000,000 in ascending order, and then ends.

```js
const { Readable } = require('stream');

class Counter extends Readable {
  constructor(opt) {
    super(opt);
    this._max = 1000000;
    this._index = 1;
  }

  _read() {
    const i = this._index++;
    if (i > this._max)
      this.push(null);
    else {
      const str = String(i);
      const buf = Buffer.from(str, 'ascii');
      this.push(buf);
    }
  }
}
```

### Implementing a duplex stream

A [`Duplex`][] stream is one that implements both [`Readable`][] and
[`Writable`][], such as a TCP socket connection.

Because JavaScript does not have support for multiple inheritance, the
`stream.Duplex` class is extended to implement a [`Duplex`][] stream (as opposed
to extending the `stream.Readable` _and_ `stream.Writable` classes).

The `stream.Duplex` class prototypically inherits from `stream.Readable` and
parasitically from `stream.Writable`, but `instanceof` will work properly for
both base classes due to overriding [`Symbol.hasInstance`][] on
`stream.Writable`.

Custom `Duplex` streams _must_ call the `new stream.Duplex([options])`
constructor and implement _both_ the [`readable._read()`][] and
`writable._write()` methods.

#### `new stream.Duplex(options)`

<!-- YAML
changes:
  - version: v8.4.0
    pr-url: https://github.com/nodejs/node/pull/14636
    description: The `readableHighWaterMark` and `writableHighWaterMark` options
                 are supported now.
-->

* `options` {Object} Passed to both `Writable` and `Readable`
  constructors. Also has the following fields:
  * `allowHalfOpen` {boolean} If set to `false`, then the stream will
    automatically end the writable side when the readable side ends.
    **Default:** `true`.
  * `readable` {boolean} Sets whether the `Duplex` should be readable.
    **Default:** `true`.
  * `writable` {boolean} Sets whether the `Duplex` should be writable.
    **Default:** `true`.
  * `readableObjectMode` {boolean} Sets `objectMode` for readable side of the
    stream. Has no effect if `objectMode` is `true`. **Default:** `false`.
  * `writableObjectMode` {boolean} Sets `objectMode` for writable side of the
    stream. Has no effect if `objectMode` is `true`. **Default:** `false`.
  * `readableHighWaterMark` {number} Sets `highWaterMark` for the readable side
    of the stream. Has no effect if `highWaterMark` is provided.
  * `writableHighWaterMark` {number} Sets `highWaterMark` for the writable side
    of the stream. Has no effect if `highWaterMark` is provided.

<!-- eslint-disable no-useless-constructor -->

```js
const { Duplex } = require('stream');

class MyDuplex extends Duplex {
  constructor(options) {
    super(options);
    // ...
  }
}
```

Or, when using pre-ES6 style constructors:

```js
const { Duplex } = require('stream');
const util = require('util');

function MyDuplex(options) {
  if (!(this instanceof MyDuplex))
    return new MyDuplex(options);
  Duplex.call(this, options);
}
util.inherits(MyDuplex, Duplex);
```

Or, using the simplified constructor approach:

```js
const { Duplex } = require('stream');

const myDuplex = new Duplex({
  read(size) {
    // ...
  },
  write(chunk, encoding, callback) {
    // ...
  }
});
```

When using pipeline:

```js
const { Transform, pipeline } = require('stream');
const fs = require('fs');

pipeline(
  fs.createReadStream('object.json')
    .setEncoding('utf8'),
  new Transform({
    decodeStrings: false, // Accept string input rather than Buffers
    construct(callback) {
      this.data = '';
      callback();
    },
    transform(chunk, encoding, callback) {
      this.data += chunk;
      callback();
    },
    flush(callback) {
      try {
        // Make sure is valid json.
        JSON.parse(this.data);
        this.push(this.data);
        callback();
      } catch (err) {
        callback(err);
      }
    }
  }),
  fs.createWriteStream('valid-object.json'),
  (err) => {
    if (err) {
      console.error('failed', err);
    } else {
      console.log('completed');
    }
  }
);
```

#### An example duplex stream

The following illustrates a simple example of a `Duplex` stream that wraps a
hypothetical lower-level source object to which data can be written, and
from which data can be read, albeit using an API that is not compatible with
Node.js streams.
The following illustrates a simple example of a `Duplex` stream that buffers
incoming written data via the [`Writable`][] interface that is read back out
via the [`Readable`][] interface.

```js
const { Duplex } = require('stream');
const kSource = Symbol('source');

class MyDuplex extends Duplex {
  constructor(source, options) {
    super(options);
    this[kSource] = source;
  }

  _write(chunk, encoding, callback) {
    // The underlying source only deals with strings.
    if (Buffer.isBuffer(chunk))
      chunk = chunk.toString();
    this[kSource].writeSomeData(chunk);
    callback();
  }

  _read(size) {
    this[kSource].fetchSomeData(size, (data, encoding) => {
      this.push(Buffer.from(data, encoding));
    });
  }
}
```

The most important aspect of a `Duplex` stream is that the `Readable` and
`Writable` sides operate independently of one another despite co-existing within
a single object instance.

#### Object mode duplex streams

For `Duplex` streams, `objectMode` can be set exclusively for either the
`Readable` or `Writable` side using the `readableObjectMode` and
`writableObjectMode` options respectively.

In the following example, for instance, a new `Transform` stream (which is a
type of [`Duplex`][] stream) is created that has an object mode `Writable` side
that accepts JavaScript numbers that are converted to hexadecimal strings on
the `Readable` side.

```js
const { Transform } = require('stream');

// All Transform streams are also Duplex Streams.
const myTransform = new Transform({
  writableObjectMode: true,

  transform(chunk, encoding, callback) {
    // Coerce the chunk to a number if necessary.
    chunk |= 0;

    // Transform the chunk into something else.
    const data = chunk.toString(16);

    // Push the data onto the readable queue.
    callback(null, '0'.repeat(data.length % 2) + data);
  }
});

myTransform.setEncoding('ascii');
myTransform.on('data', (chunk) => console.log(chunk));

myTransform.write(1);
// Prints: 01
myTransform.write(10);
// Prints: 0a
myTransform.write(100);
// Prints: 64
```

### Implementing a transform stream

A [`Transform`][] stream is a [`Duplex`][] stream where the output is computed
in some way from the input. Examples include [zlib][] streams or [crypto][]
streams that compress, encrypt, or decrypt data.

There is no requirement that the output be the same size as the input, the same
number of chunks, or arrive at the same time. For example, a `Hash` stream will
only ever have a single chunk of output which is provided when the input is
ended. A `zlib` stream will produce output that is either much smaller or much
larger than its input.

The `stream.Transform` class is extended to implement a [`Transform`][] stream.

The `stream.Transform` class prototypically inherits from `stream.Duplex` and
implements its own versions of the `writable._write()` and
[`readable._read()`][] methods. Custom `Transform` implementations _must_
implement the [`transform._transform()`][stream-_transform] method and _may_
also implement the [`transform._flush()`][stream-_flush] method.

Care must be taken when using `Transform` streams in that data written to the
stream can cause the `Writable` side of the stream to become paused if the
output on the `Readable` side is not consumed.

#### `new stream.Transform([options])`

* `options` {Object} Passed to both `Writable` and `Readable`
  constructors. Also has the following fields:
  * `transform` {Function} Implementation for the
    [`stream._transform()`][stream-_transform] method.
  * `flush` {Function} Implementation for the [`stream._flush()`][stream-_flush]
    method.

<!-- eslint-disable no-useless-constructor -->

```js
const { Transform } = require('stream');

class MyTransform extends Transform {
  constructor(options) {
    super(options);
    // ...
  }
}
```

Or, when using pre-ES6 style constructors:

```js
const { Transform } = require('stream');
const util = require('util');

function MyTransform(options) {
  if (!(this instanceof MyTransform))
    return new MyTransform(options);
  Transform.call(this, options);
}
util.inherits(MyTransform, Transform);
```

Or, using the simplified constructor approach:

```js
const { Transform } = require('stream');

const myTransform = new Transform({
  transform(chunk, encoding, callback) {
    // ...
  }
});
```

#### Event: `'end'`

The [`'end'`][] event is from the `stream.Readable` class. The `'end'` event is
emitted after all data has been output, which occurs after the callback in
[`transform._flush()`][stream-_flush] has been called. In the case of an error,
`'end'` should not be emitted.

#### Event: `'finish'`

The [`'finish'`][] event is from the `stream.Writable` class. The `'finish'`
event is emitted after [`stream.end()`][stream-end] is called and all chunks
have been processed by [`stream._transform()`][stream-_transform]. In the case
of an error, `'finish'` should not be emitted.

#### `transform._flush(callback)`

* `callback` {Function} A callback function (optionally with an error
  argument and data) to be called when remaining data has been flushed.

This function MUST NOT be called by application code directly. It should be
implemented by child classes, and called by the internal `Readable` class
methods only.

In some cases, a transform operation may need to emit an additional bit of
data at the end of the stream. For example, a `zlib` compression stream will
store an amount of internal state used to optimally compress the output. When
the stream ends, however, that additional data needs to be flushed so that the
compressed data will be complete.

Custom [`Transform`][] implementations _may_ implement the `transform._flush()`
method. This will be called when there is no more written data to be consumed,
but before the [`'end'`][] event is emitted signaling the end of the
[`Readable`][] stream.

Within the `transform._flush()` implementation, the `transform.push()` method
may be called zero or more times, as appropriate. The `callback` function must
be called when the flush operation is complete.

The `transform._flush()` method is prefixed with an underscore because it is
internal to the class that defines it, and should never be called directly by
user programs.

#### `transform._transform(chunk, encoding, callback)`

* `chunk` {Buffer|string|any} The `Buffer` to be transformed, converted from
  the `string` passed to [`stream.write()`][stream-write]. If the stream's
  `decodeStrings` option is `false` or the stream is operating in object mode,
  the chunk will not be converted & will be whatever was passed to
  [`stream.write()`][stream-write].
* `encoding` {string} If the chunk is a string, then this is the
  encoding type. If chunk is a buffer, then this is the special
  value `'buffer'`. Ignore it in that case.
* `callback` {Function} A callback function (optionally with an error
  argument and data) to be called after the supplied `chunk` has been
  processed.

This function MUST NOT be called by application code directly. It should be
implemented by child classes, and called by the internal `Readable` class
methods only.

All `Transform` stream implementations must provide a `_transform()`
method to accept input and produce output. The `transform._transform()`
implementation handles the bytes being written, computes an output, then passes
that output off to the readable portion using the `transform.push()` method.

The `transform.push()` method may be called zero or more times to generate
output from a single input chunk, depending on how much is to be output
as a result of the chunk.

It is possible that no output is generated from any given chunk of input data.

The `callback` function must be called only when the current chunk is completely
consumed. The first argument passed to the `callback` must be an `Error` object
if an error occurred while processing the input or `null` otherwise. If a second
argument is passed to the `callback`, it will be forwarded on to the
`transform.push()` method. In other words, the following are equivalent:

```js
transform.prototype._transform = function(data, encoding, callback) {
  this.push(data);
  callback();
};

transform.prototype._transform = function(data, encoding, callback) {
  callback(null, data);
};
```

The `transform._transform()` method is prefixed with an underscore because it
is internal to the class that defines it, and should never be called directly by
user programs.

`transform._transform()` is never called in parallel; streams implement a
queue mechanism, and to receive the next chunk, `callback` must be
called, either synchronously or asynchronously.

#### Class: `stream.PassThrough`

The `stream.PassThrough` class is a trivial implementation of a [`Transform`][]
stream that simply passes the input bytes across to the output. Its purpose is
primarily for examples and testing, but there are some use cases where
`stream.PassThrough` is useful as a building block for novel sorts of streams.

## Additional notes

<!--type=misc-->

### Streams compatibility with async generators and async iterators

With the support of async generators and iterators in JavaScript, async
generators are effectively a first-class language-level stream construct at
this point.

Some common interop cases of using Node.js streams with async generators
and async iterators are provided below.

#### Consuming readable streams with async iterators

```js
(async function() {
  for await (const chunk of readable) {
    console.log(chunk);
  }
})();
```

Async iterators register a permanent error handler on the stream to prevent any
unhandled post-destroy errors.

#### Creating readable streams with async generators

A Node.js readable stream can be created from an asynchronous generator using
the `Readable.from()` utility method:

```js
const { Readable } = require('stream');

const ac = new AbortController();
const signal = ac.signal;

async function * generate() {
  yield 'a';
  await someLongRunningFn({ signal });
  yield 'b';
  yield 'c';
}

const readable = Readable.from(generate());
readable.on('close', () => {
  ac.abort();
});

readable.on('data', (chunk) => {
  console.log(chunk);
});
```

#### Piping to writable streams from async iterators

When writing to a writable stream from an async iterator, ensure correct
handling of backpressure and errors. [`stream.pipeline()`][] abstracts away
the handling of backpressure and backpressure-related errors:

```js
const fs = require('fs');
const { pipeline } = require('stream');
const { pipeline: pipelinePromise } = require('stream/promises');

const writable = fs.createWriteStream('./file');

const ac = new AbortController();
const signal = ac.signal;

const iterator = createIterator({ signal });

// Callback Pattern
pipeline(iterator, writable, (err, value) => {
  if (err) {
    console.error(err);
  } else {
    console.log(value, 'value returned');
  }
}).on('close', () => {
  ac.abort();
});

// Promise Pattern
pipelinePromise(iterator, writable)
  .then((value) => {
    console.log(value, 'value returned');
  })
  .catch((err) => {
    console.error(err);
    ac.abort();
  });
```

<!--type=misc-->

### Compatibility with older Node.js versions

<!--type=misc-->

Prior to Node.js 0.10, the `Readable` stream interface was simpler, but also
less powerful and less useful.

* Rather than waiting for calls to the [`stream.read()`][stream-read] method,
  [`'data'`][] events would begin emitting immediately. Applications that
  would need to perform some amount of work to decide how to handle data
  were required to store read data into buffers so the data would not be lost.
* The [`stream.pause()`][stream-pause] method was advisory, rather than
  guaranteed. This meant that it was still necessary to be prepared to receive
  [`'data'`][] events _even when the stream was in a paused state_.

In Node.js 0.10, the [`Readable`][] class was added. For backward
compatibility with older Node.js programs, `Readable` streams switch into
"flowing mode" when a [`'data'`][] event handler is added, or when the
[`stream.resume()`][stream-resume] method is called. The effect is that, even
when not using the new [`stream.read()`][stream-read] method and
[`'readable'`][] event, it is no longer necessary to worry about losing
[`'data'`][] chunks.

While most applications will continue to function normally, this introduces an
edge case in the following conditions:

* No [`'data'`][] event listener is added.
* The [`stream.resume()`][stream-resume] method is never called.
* The stream is not piped to any writable destination.

For example, consider the following code:

```js
// WARNING!  BROKEN!
net.createServer((socket) => {

  // We add an 'end' listener, but never consume the data.
  socket.on('end', () => {
    // It will never get here.
    socket.end('The message was received but was not processed.\n');
  });

}).listen(1337);
```

Prior to Node.js 0.10, the incoming message data would be simply discarded.
However, in Node.js 0.10 and beyond, the socket remains paused forever.

The workaround in this situation is to call the
[`stream.resume()`][stream-resume] method to begin the flow of data:

```js
// Workaround.
net.createServer((socket) => {
  socket.on('end', () => {
    socket.end('The message was received but was not processed.\n');
  });

  // Start the flow of data, discarding it.
  socket.resume();
}).listen(1337);
```

In addition to new `Readable` streams switching into flowing mode,
pre-0.10 style streams can be wrapped in a `Readable` class using the
[`readable.wrap()`][`stream.wrap()`] method.

### `readable.read(0)`

There are some cases where it is necessary to trigger a refresh of the
underlying readable stream mechanisms, without actually consuming any
data. In such cases, it is possible to call `readable.read(0)`, which will
always return `null`.

If the internal read buffer is below the `highWaterMark`, and the
stream is not currently reading, then calling `stream.read(0)` will trigger
a low-level [`stream._read()`][stream-_read] call.

While most applications will almost never need to do this, there are
situations within Node.js where this is done, particularly in the
`Readable` stream class internals.

### `readable.push('')`

Use of `readable.push('')` is not recommended.

Pushing a zero-byte string, `Buffer` or `Uint8Array` to a stream that is not in
object mode has an interesting side effect. Because it _is_ a call to
[`readable.push()`][stream-push], the call will end the reading process.
However, because the argument is an empty string, no data is added to the
readable buffer so there is nothing for a user to consume.

### `highWaterMark` discrepancy after calling `readable.setEncoding()`

The use of `readable.setEncoding()` will change the behavior of how the
`highWaterMark` operates in non-object mode.

Typically, the size of the current buffer is measured against the
`highWaterMark` in _bytes_. However, after `setEncoding()` is called, the
comparison function will begin to measure the buffer's size in _characters_.

This is not a problem in common cases with `latin1` or `ascii`. But it is
advised to be mindful about this behavior when working with strings that could
contain multi-byte characters.

[API for stream consumers]: #api-for-stream-consumers
[API for stream implementers]: #api-for-stream-implementers
[Compatibility]: #compatibility-with-older-nodejs-versions
[HTTP requests, on the client]: http.md#class-httpclientrequest
[HTTP responses, on the server]: http.md#class-httpserverresponse
[TCP sockets]: net.md#class-netsocket
[Three states]: #three-states
[`'data'`]: #event-data
[`'drain'`]: #event-drain
[`'end'`]: #event-end
[`'finish'`]: #event-finish
[`'readable'`]: #event-readable
[`Duplex`]: #class-streamduplex
[`EventEmitter`]: events.md#class-eventemitter
[`Readable`]: #class-streamreadable
[`Symbol.hasInstance`]: https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Symbol/hasInstance
[`Transform`]: #class-streamtransform
[`Writable`]: #class-streamwritable
[`fs.createReadStream()`]: fs.md#fscreatereadstreampath-options
[`fs.createWriteStream()`]: fs.md#fscreatewritestreampath-options
[`net.Socket`]: net.md#class-netsocket
[`process.stderr`]: process.md#processstderr
[`process.stdin`]: process.md#processstdin
[`process.stdout`]: process.md#processstdout
[`readable._read()`]: #readable_readsize
[`readable.push('')`]: #readablepush
[`readable.setEncoding()`]: #readablesetencodingencoding
[`stream.Readable.from()`]: #streamreadablefromiterable-options
[`stream.addAbortSignal()`]: #streamaddabortsignalsignal-stream
[`stream.cork()`]: #writablecork
[`stream.finished()`]: #streamfinishedstream-options-callback
[`stream.pipe()`]: #readablepipedestination-options
[`stream.pipeline()`]: #streampipelinesource-transforms-destination-callback
[`stream.uncork()`]: #writableuncork
[`stream.unpipe()`]: #readableunpipedestination
[`stream.wrap()`]: #readablewrapstream
[`writable._final()`]: #writable_finalcallback
[`writable._write()`]: #writable_writechunk-encoding-callback
[`writable._writev()`]: #writable_writevchunks-callback
[`writable.cork()`]: #writablecork
[`writable.end()`]: #writableendchunk-encoding-callback
[`writable.uncork()`]: #writableuncork
[`writable.writableFinished`]: #writablewritablefinished
[`zlib.createDeflate()`]: zlib.md#zlibcreatedeflateoptions
[child process stdin]: child_process.md#subprocessstdin
[child process stdout and stderr]: child_process.md#subprocessstdout
[crypto]: crypto.md
[fs read streams]: fs.md#class-fsreadstream
[fs write streams]: fs.md#class-fswritestream
[http-incoming-message]: http.md#class-httpincomingmessage
[hwm-gotcha]: #highwatermark-discrepancy-after-calling-readablesetencoding
[object-mode]: #object-mode
[readable-_construct]: #readable_constructcallback
[readable-_destroy]: #readable_destroyerr-callback
[readable-destroy]: #readabledestroyerror
[stream-_final]: #writable_finalcallback
[stream-_flush]: #transform_flushcallback
[stream-_read]: #readable_readsize
[stream-_transform]: #transform_transformchunk-encoding-callback
[stream-_write]: #writable_writechunk-encoding-callback
[stream-_writev]: #writable_writevchunks-callback
[stream-end]: #writableendchunk-encoding-callback
[stream-pause]: #readablepause
[stream-push]: #readablepushchunk-encoding
[stream-read]: #readablereadsize
[stream-resume]: #readableresume
[stream-uncork]: #writableuncork
[stream-write]: #writablewritechunk-encoding-callback
[writable-_construct]: #writable_constructcallback
[writable-_destroy]: #writable_destroyerr-callback
[writable-destroy]: #writabledestroyerror
[writable-new]: #new-streamwritableoptions
[zlib]: zlib.md