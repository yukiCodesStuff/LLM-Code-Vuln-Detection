port1.onmessage = ({ data }) => {
  const { writable, readable } = data;
  // ...
};

port2.postMessage(stream, [stream]);
```

### Class: `TransformStreamDefaultController`
<!-- YAML
added: v16.5.0
-->

The `TransformStreamDefaultController` manages the internal state
of the `TransformStream`.

#### `transformStreamDefaultController.desiredSize`
<!-- YAML
added: v16.5.0
-->

* Type: {number}

The amount of data required to fill the readable side's queue.

#### `transformStreamDefaultController.enqueue([chunk])`
<!-- YAML
added: v16.5.0
-->

* `chunk` {any}

Appends a chunk of data to the readable side's queue.

#### `transformStreamDefaultController.error([reason])`
<!-- YAML
added: v16.5.0
-->

* `reason` {any}

Signals to both the readable and writable side that an error has occurred
while processing the transform data, causing both sides to be abruptly
closed.

#### `transformStreamDefaultController.terminate()`
<!-- YAML
added: v16.5.0
-->

Closes the readable side of the transport and causes the writable side
to be abruptly closed with an error.

### Class: `ByteLengthQueuingStrategy`
<!-- YAML
added: v16.5.0
-->

#### `new ByteLengthQueuingStrategy(options)`
<!-- YAML
added: v16.5.0
-->

* `options` {Object}
  * `highWaterMark` {number}

#### `byteLengthQueuingStrategy.highWaterMark`
<!-- YAML
added: v16.5.0
-->

* Type: {number}

#### `byteLengthQueuingStrategy.size`
<!-- YAML
added: v16.5.0
-->

* Type: {Function}
  * `chunk` {any}
  * Returns: {number}

### Class: `CountQueuingStrategy`
<!-- YAML
added: v16.5.0
-->

#### `new CountQueuingStrategy(options)`
<!-- YAML
added: v16.5.0
-->

* `options` {Object}
  * `highWaterMark` {number}

#### `countQueuingStrategy.highWaterMark`
<!-- YAML
added: v16.5.0
-->

* Type: {number}

#### `countQueuingStrategy.size`
<!-- YAML
added: v16.5.0
-->

* Type: {Function}
  * `chunk` {any}
  * Returns: {number}

### Class: `TextEncoderStream`
<!-- YAML
added: v16.6.0
-->

#### `new TextEncoderStream()`
<!-- YAML
added: v16.6.0
-->

Creates a new `TextEncoderStream` instance.

#### `textEncoderStream.encoding`
<!-- YAML
added: v16.6.0
-->

* Type: {string}

The encoding supported by the `TextEncoderStream` instance.

#### `textEncoderStream.readable`
<!-- YAML
added: v16.6.0
-->

* Type: {ReadableStream}

#### `textEncoderStream.writable`
<!-- YAML
added: v16.6.0
-->

* Type: {WritableStream}

### Class: `TextDecoderStream`
<!-- YAML
added: v16.6.0
-->

#### `new TextDecoderStream([encoding[, options]])`
<!-- YAML
added: v16.6.0
-->

* `encoding` {string} Identifies the `encoding` that this `TextDecoder` instance
  supports. **Default:** `'utf-8'`.
* `options` {Object}
  * `fatal` {boolean} `true` if decoding failures are fatal.
  * `ignoreBOM` {boolean} When `true`, the `TextDecoderStream` will include the
    byte order mark in the decoded result. When `false`, the byte order mark
    will be removed from the output. This option is only used when `encoding` is
    `'utf-8'`, `'utf-16be'` or `'utf-16le'`. **Default:** `false`.

Creates a new `TextDecoderStream` instance.

#### `textDecoderStream.encoding`
<!-- YAML
added: v16.6.0
-->

* Type: {string}

The encoding supported by the `TextDecoderStream` instance.

#### `textDecoderStream.fatal`
<!-- YAML
added: v16.6.0
-->

* Type: {boolean}

The value will be `true` if decoding errors result in a `TypeError` being
thrown.

#### `textDecoderStream.ignoreBOM`
<!-- YAML
added: v16.6.0
-->

* Type: {boolean}

The value will be `true` if the decoding result will include the byte order
mark.

#### `textDecoderStream.readable`
<!-- YAML
added: v16.6.0
-->

* Type: {ReadableStream}

#### `textDecoderStream.writable`
<!-- YAML
added: v16.6.0
-->

* Type: {WritableStream}

### Class: `CompressionStream`
<!-- YAML
added: REPLACEME
-->
#### `new CompressionStream(format)`
<!-- YAML
added: REPLACEME
-->

* `format` {string} One of either `'deflate'` or `'gzip'`.

#### `compressionStream.readable`
<!-- YAML
added: REPLACEME
-->

* Type: {ReadableStream}

#### `compressionStream.writable`
<!-- YAML
added: REPLACEME
-->

* Type: {WritableStream}

### Class: `DecompressionStream`
<!-- YAML
added: REPLACEME
-->

#### `new DecompressionStream(format)`
<!-- YAML
added: REPLACEME
-->

* `format` {string} One of either `'deflate'` or `'gzip'`.

#### `decompressionStream.readable`
<!-- YAML
added: REPLACEME
-->

* Type: {ReadableStream}

#### `deccompressionStream.writable`
<!-- YAML
added: REPLACEME
-->

* Type: {WritableStream}

[Streams]: stream.md
[WHATWG Streams Standard]: https://streams.spec.whatwg.org/
port1.onmessage = ({ data }) => {
  const { writable, readable } = data;
  // ...
};

port2.postMessage(stream, [stream]);
```

### Class: `TransformStreamDefaultController`
<!-- YAML
added: v16.5.0
-->

The `TransformStreamDefaultController` manages the internal state
of the `TransformStream`.

#### `transformStreamDefaultController.desiredSize`
<!-- YAML
added: v16.5.0
-->

* Type: {number}

The amount of data required to fill the readable side's queue.

#### `transformStreamDefaultController.enqueue([chunk])`
<!-- YAML
added: v16.5.0
-->

* `chunk` {any}

Appends a chunk of data to the readable side's queue.

#### `transformStreamDefaultController.error([reason])`
<!-- YAML
added: v16.5.0
-->

* `reason` {any}

Signals to both the readable and writable side that an error has occurred
while processing the transform data, causing both sides to be abruptly
closed.

#### `transformStreamDefaultController.terminate()`
<!-- YAML
added: v16.5.0
-->

Closes the readable side of the transport and causes the writable side
to be abruptly closed with an error.

### Class: `ByteLengthQueuingStrategy`
<!-- YAML
added: v16.5.0
-->

#### `new ByteLengthQueuingStrategy(options)`
<!-- YAML
added: v16.5.0
-->

* `options` {Object}
  * `highWaterMark` {number}

#### `byteLengthQueuingStrategy.highWaterMark`
<!-- YAML
added: v16.5.0
-->

* Type: {number}

#### `byteLengthQueuingStrategy.size`
<!-- YAML
added: v16.5.0
-->

* Type: {Function}
  * `chunk` {any}
  * Returns: {number}

### Class: `CountQueuingStrategy`
<!-- YAML
added: v16.5.0
-->

#### `new CountQueuingStrategy(options)`
<!-- YAML
added: v16.5.0
-->

* `options` {Object}
  * `highWaterMark` {number}

#### `countQueuingStrategy.highWaterMark`
<!-- YAML
added: v16.5.0
-->

* Type: {number}

#### `countQueuingStrategy.size`
<!-- YAML
added: v16.5.0
-->

* Type: {Function}
  * `chunk` {any}
  * Returns: {number}

### Class: `TextEncoderStream`
<!-- YAML
added: v16.6.0
-->

#### `new TextEncoderStream()`
<!-- YAML
added: v16.6.0
-->

Creates a new `TextEncoderStream` instance.

#### `textEncoderStream.encoding`
<!-- YAML
added: v16.6.0
-->

* Type: {string}

The encoding supported by the `TextEncoderStream` instance.

#### `textEncoderStream.readable`
<!-- YAML
added: v16.6.0
-->

* Type: {ReadableStream}

#### `textEncoderStream.writable`
<!-- YAML
added: v16.6.0
-->

* Type: {WritableStream}

### Class: `TextDecoderStream`
<!-- YAML
added: v16.6.0
-->

#### `new TextDecoderStream([encoding[, options]])`
<!-- YAML
added: v16.6.0
-->

* `encoding` {string} Identifies the `encoding` that this `TextDecoder` instance
  supports. **Default:** `'utf-8'`.
* `options` {Object}
  * `fatal` {boolean} `true` if decoding failures are fatal.
  * `ignoreBOM` {boolean} When `true`, the `TextDecoderStream` will include the
    byte order mark in the decoded result. When `false`, the byte order mark
    will be removed from the output. This option is only used when `encoding` is
    `'utf-8'`, `'utf-16be'` or `'utf-16le'`. **Default:** `false`.

Creates a new `TextDecoderStream` instance.

#### `textDecoderStream.encoding`
<!-- YAML
added: v16.6.0
-->

* Type: {string}

The encoding supported by the `TextDecoderStream` instance.

#### `textDecoderStream.fatal`
<!-- YAML
added: v16.6.0
-->

* Type: {boolean}

The value will be `true` if decoding errors result in a `TypeError` being
thrown.

#### `textDecoderStream.ignoreBOM`
<!-- YAML
added: v16.6.0
-->

* Type: {boolean}

The value will be `true` if the decoding result will include the byte order
mark.

#### `textDecoderStream.readable`
<!-- YAML
added: v16.6.0
-->

* Type: {ReadableStream}

#### `textDecoderStream.writable`
<!-- YAML
added: v16.6.0
-->

* Type: {WritableStream}

### Class: `CompressionStream`
<!-- YAML
added: REPLACEME
-->
#### `new CompressionStream(format)`
<!-- YAML
added: REPLACEME
-->

* `format` {string} One of either `'deflate'` or `'gzip'`.

#### `compressionStream.readable`
<!-- YAML
added: REPLACEME
-->

* Type: {ReadableStream}

#### `compressionStream.writable`
<!-- YAML
added: REPLACEME
-->

* Type: {WritableStream}

### Class: `DecompressionStream`
<!-- YAML
added: REPLACEME
-->

#### `new DecompressionStream(format)`
<!-- YAML
added: REPLACEME
-->

* `format` {string} One of either `'deflate'` or `'gzip'`.

#### `decompressionStream.readable`
<!-- YAML
added: REPLACEME
-->

* Type: {ReadableStream}

#### `deccompressionStream.writable`
<!-- YAML
added: REPLACEME
-->

* Type: {WritableStream}

[Streams]: stream.md
[WHATWG Streams Standard]: https://streams.spec.whatwg.org/
port1.onmessage = ({ data }) => {
  const { writable, readable } = data;
  // ...
};

port2.postMessage(stream, [stream]);
```

### Class: `TransformStreamDefaultController`
<!-- YAML
added: v16.5.0
-->

The `TransformStreamDefaultController` manages the internal state
of the `TransformStream`.

#### `transformStreamDefaultController.desiredSize`
<!-- YAML
added: v16.5.0
-->

* Type: {number}

The amount of data required to fill the readable side's queue.

#### `transformStreamDefaultController.enqueue([chunk])`
<!-- YAML
added: v16.5.0
-->

* `chunk` {any}

Appends a chunk of data to the readable side's queue.

#### `transformStreamDefaultController.error([reason])`
<!-- YAML
added: v16.5.0
-->

* `reason` {any}

Signals to both the readable and writable side that an error has occurred
while processing the transform data, causing both sides to be abruptly
closed.

#### `transformStreamDefaultController.terminate()`
<!-- YAML
added: v16.5.0
-->

Closes the readable side of the transport and causes the writable side
to be abruptly closed with an error.

### Class: `ByteLengthQueuingStrategy`
<!-- YAML
added: v16.5.0
-->

#### `new ByteLengthQueuingStrategy(options)`
<!-- YAML
added: v16.5.0
-->

* `options` {Object}
  * `highWaterMark` {number}

#### `byteLengthQueuingStrategy.highWaterMark`
<!-- YAML
added: v16.5.0
-->

* Type: {number}

#### `byteLengthQueuingStrategy.size`
<!-- YAML
added: v16.5.0
-->

* Type: {Function}
  * `chunk` {any}
  * Returns: {number}

### Class: `CountQueuingStrategy`
<!-- YAML
added: v16.5.0
-->

#### `new CountQueuingStrategy(options)`
<!-- YAML
added: v16.5.0
-->

* `options` {Object}
  * `highWaterMark` {number}

#### `countQueuingStrategy.highWaterMark`
<!-- YAML
added: v16.5.0
-->

* Type: {number}

#### `countQueuingStrategy.size`
<!-- YAML
added: v16.5.0
-->

* Type: {Function}
  * `chunk` {any}
  * Returns: {number}

### Class: `TextEncoderStream`
<!-- YAML
added: v16.6.0
-->

#### `new TextEncoderStream()`
<!-- YAML
added: v16.6.0
-->

Creates a new `TextEncoderStream` instance.

#### `textEncoderStream.encoding`
<!-- YAML
added: v16.6.0
-->

* Type: {string}

The encoding supported by the `TextEncoderStream` instance.

#### `textEncoderStream.readable`
<!-- YAML
added: v16.6.0
-->

* Type: {ReadableStream}

#### `textEncoderStream.writable`
<!-- YAML
added: v16.6.0
-->

* Type: {WritableStream}

### Class: `TextDecoderStream`
<!-- YAML
added: v16.6.0
-->

#### `new TextDecoderStream([encoding[, options]])`
<!-- YAML
added: v16.6.0
-->

* `encoding` {string} Identifies the `encoding` that this `TextDecoder` instance
  supports. **Default:** `'utf-8'`.
* `options` {Object}
  * `fatal` {boolean} `true` if decoding failures are fatal.
  * `ignoreBOM` {boolean} When `true`, the `TextDecoderStream` will include the
    byte order mark in the decoded result. When `false`, the byte order mark
    will be removed from the output. This option is only used when `encoding` is
    `'utf-8'`, `'utf-16be'` or `'utf-16le'`. **Default:** `false`.

Creates a new `TextDecoderStream` instance.

#### `textDecoderStream.encoding`
<!-- YAML
added: v16.6.0
-->

* Type: {string}

The encoding supported by the `TextDecoderStream` instance.

#### `textDecoderStream.fatal`
<!-- YAML
added: v16.6.0
-->

* Type: {boolean}

The value will be `true` if decoding errors result in a `TypeError` being
thrown.

#### `textDecoderStream.ignoreBOM`
<!-- YAML
added: v16.6.0
-->

* Type: {boolean}

The value will be `true` if the decoding result will include the byte order
mark.

#### `textDecoderStream.readable`
<!-- YAML
added: v16.6.0
-->

* Type: {ReadableStream}

#### `textDecoderStream.writable`
<!-- YAML
added: v16.6.0
-->

* Type: {WritableStream}

### Class: `CompressionStream`
<!-- YAML
added: REPLACEME
-->
#### `new CompressionStream(format)`
<!-- YAML
added: REPLACEME
-->

* `format` {string} One of either `'deflate'` or `'gzip'`.

#### `compressionStream.readable`
<!-- YAML
added: REPLACEME
-->

* Type: {ReadableStream}

#### `compressionStream.writable`
<!-- YAML
added: REPLACEME
-->

* Type: {WritableStream}

### Class: `DecompressionStream`
<!-- YAML
added: REPLACEME
-->

#### `new DecompressionStream(format)`
<!-- YAML
added: REPLACEME
-->

* `format` {string} One of either `'deflate'` or `'gzip'`.

#### `decompressionStream.readable`
<!-- YAML
added: REPLACEME
-->

* Type: {ReadableStream}

#### `deccompressionStream.writable`
<!-- YAML
added: REPLACEME
-->

* Type: {WritableStream}

[Streams]: stream.md
[WHATWG Streams Standard]: https://streams.spec.whatwg.org/
port1.onmessage = ({ data }) => {
  const { writable, readable } = data;
  // ...
};

port2.postMessage(stream, [stream]);
```

### Class: `TransformStreamDefaultController`
<!-- YAML
added: v16.5.0
-->

The `TransformStreamDefaultController` manages the internal state
of the `TransformStream`.

#### `transformStreamDefaultController.desiredSize`
<!-- YAML
added: v16.5.0
-->

* Type: {number}

The amount of data required to fill the readable side's queue.

#### `transformStreamDefaultController.enqueue([chunk])`
<!-- YAML
added: v16.5.0
-->

* `chunk` {any}

Appends a chunk of data to the readable side's queue.

#### `transformStreamDefaultController.error([reason])`
<!-- YAML
added: v16.5.0
-->

* `reason` {any}

Signals to both the readable and writable side that an error has occurred
while processing the transform data, causing both sides to be abruptly
closed.

#### `transformStreamDefaultController.terminate()`
<!-- YAML
added: v16.5.0
-->

Closes the readable side of the transport and causes the writable side
to be abruptly closed with an error.

### Class: `ByteLengthQueuingStrategy`
<!-- YAML
added: v16.5.0
-->

#### `new ByteLengthQueuingStrategy(options)`
<!-- YAML
added: v16.5.0
-->

* `options` {Object}
  * `highWaterMark` {number}

#### `byteLengthQueuingStrategy.highWaterMark`
<!-- YAML
added: v16.5.0
-->

* Type: {number}

#### `byteLengthQueuingStrategy.size`
<!-- YAML
added: v16.5.0
-->

* Type: {Function}
  * `chunk` {any}
  * Returns: {number}

### Class: `CountQueuingStrategy`
<!-- YAML
added: v16.5.0
-->

#### `new CountQueuingStrategy(options)`
<!-- YAML
added: v16.5.0
-->

* `options` {Object}
  * `highWaterMark` {number}

#### `countQueuingStrategy.highWaterMark`
<!-- YAML
added: v16.5.0
-->

* Type: {number}

#### `countQueuingStrategy.size`
<!-- YAML
added: v16.5.0
-->

* Type: {Function}
  * `chunk` {any}
  * Returns: {number}

### Class: `TextEncoderStream`
<!-- YAML
added: v16.6.0
-->

#### `new TextEncoderStream()`
<!-- YAML
added: v16.6.0
-->

Creates a new `TextEncoderStream` instance.

#### `textEncoderStream.encoding`
<!-- YAML
added: v16.6.0
-->

* Type: {string}

The encoding supported by the `TextEncoderStream` instance.

#### `textEncoderStream.readable`
<!-- YAML
added: v16.6.0
-->

* Type: {ReadableStream}

#### `textEncoderStream.writable`
<!-- YAML
added: v16.6.0
-->

* Type: {WritableStream}

### Class: `TextDecoderStream`
<!-- YAML
added: v16.6.0
-->

#### `new TextDecoderStream([encoding[, options]])`
<!-- YAML
added: v16.6.0
-->

* `encoding` {string} Identifies the `encoding` that this `TextDecoder` instance
  supports. **Default:** `'utf-8'`.
* `options` {Object}
  * `fatal` {boolean} `true` if decoding failures are fatal.
  * `ignoreBOM` {boolean} When `true`, the `TextDecoderStream` will include the
    byte order mark in the decoded result. When `false`, the byte order mark
    will be removed from the output. This option is only used when `encoding` is
    `'utf-8'`, `'utf-16be'` or `'utf-16le'`. **Default:** `false`.

Creates a new `TextDecoderStream` instance.

#### `textDecoderStream.encoding`
<!-- YAML
added: v16.6.0
-->

* Type: {string}

The encoding supported by the `TextDecoderStream` instance.

#### `textDecoderStream.fatal`
<!-- YAML
added: v16.6.0
-->

* Type: {boolean}

The value will be `true` if decoding errors result in a `TypeError` being
thrown.

#### `textDecoderStream.ignoreBOM`
<!-- YAML
added: v16.6.0
-->

* Type: {boolean}

The value will be `true` if the decoding result will include the byte order
mark.

#### `textDecoderStream.readable`
<!-- YAML
added: v16.6.0
-->

* Type: {ReadableStream}

#### `textDecoderStream.writable`
<!-- YAML
added: v16.6.0
-->

* Type: {WritableStream}

### Class: `CompressionStream`
<!-- YAML
added: REPLACEME
-->
#### `new CompressionStream(format)`
<!-- YAML
added: REPLACEME
-->

* `format` {string} One of either `'deflate'` or `'gzip'`.

#### `compressionStream.readable`
<!-- YAML
added: REPLACEME
-->

* Type: {ReadableStream}

#### `compressionStream.writable`
<!-- YAML
added: REPLACEME
-->

* Type: {WritableStream}

### Class: `DecompressionStream`
<!-- YAML
added: REPLACEME
-->

#### `new DecompressionStream(format)`
<!-- YAML
added: REPLACEME
-->

* `format` {string} One of either `'deflate'` or `'gzip'`.

#### `decompressionStream.readable`
<!-- YAML
added: REPLACEME
-->

* Type: {ReadableStream}

#### `deccompressionStream.writable`
<!-- YAML
added: REPLACEME
-->

* Type: {WritableStream}

[Streams]: stream.md
[WHATWG Streams Standard]: https://streams.spec.whatwg.org/
port1.onmessage = ({ data }) => {
  const { writable, readable } = data;
  // ...
};

port2.postMessage(stream, [stream]);
```

### Class: `TransformStreamDefaultController`
<!-- YAML
added: v16.5.0
-->

The `TransformStreamDefaultController` manages the internal state
of the `TransformStream`.

#### `transformStreamDefaultController.desiredSize`
<!-- YAML
added: v16.5.0
-->

* Type: {number}

The amount of data required to fill the readable side's queue.

#### `transformStreamDefaultController.enqueue([chunk])`
<!-- YAML
added: v16.5.0
-->

* `chunk` {any}

Appends a chunk of data to the readable side's queue.

#### `transformStreamDefaultController.error([reason])`
<!-- YAML
added: v16.5.0
-->

* `reason` {any}

Signals to both the readable and writable side that an error has occurred
while processing the transform data, causing both sides to be abruptly
closed.

#### `transformStreamDefaultController.terminate()`
<!-- YAML
added: v16.5.0
-->

Closes the readable side of the transport and causes the writable side
to be abruptly closed with an error.

### Class: `ByteLengthQueuingStrategy`
<!-- YAML
added: v16.5.0
-->

#### `new ByteLengthQueuingStrategy(options)`
<!-- YAML
added: v16.5.0
-->

* `options` {Object}
  * `highWaterMark` {number}

#### `byteLengthQueuingStrategy.highWaterMark`
<!-- YAML
added: v16.5.0
-->

* Type: {number}

#### `byteLengthQueuingStrategy.size`
<!-- YAML
added: v16.5.0
-->

* Type: {Function}
  * `chunk` {any}
  * Returns: {number}

### Class: `CountQueuingStrategy`
<!-- YAML
added: v16.5.0
-->

#### `new CountQueuingStrategy(options)`
<!-- YAML
added: v16.5.0
-->

* `options` {Object}
  * `highWaterMark` {number}

#### `countQueuingStrategy.highWaterMark`
<!-- YAML
added: v16.5.0
-->

* Type: {number}

#### `countQueuingStrategy.size`
<!-- YAML
added: v16.5.0
-->

* Type: {Function}
  * `chunk` {any}
  * Returns: {number}

### Class: `TextEncoderStream`
<!-- YAML
added: v16.6.0
-->

#### `new TextEncoderStream()`
<!-- YAML
added: v16.6.0
-->

Creates a new `TextEncoderStream` instance.

#### `textEncoderStream.encoding`
<!-- YAML
added: v16.6.0
-->

* Type: {string}

The encoding supported by the `TextEncoderStream` instance.

#### `textEncoderStream.readable`
<!-- YAML
added: v16.6.0
-->

* Type: {ReadableStream}

#### `textEncoderStream.writable`
<!-- YAML
added: v16.6.0
-->

* Type: {WritableStream}

### Class: `TextDecoderStream`
<!-- YAML
added: v16.6.0
-->

#### `new TextDecoderStream([encoding[, options]])`
<!-- YAML
added: v16.6.0
-->

* `encoding` {string} Identifies the `encoding` that this `TextDecoder` instance
  supports. **Default:** `'utf-8'`.
* `options` {Object}
  * `fatal` {boolean} `true` if decoding failures are fatal.
  * `ignoreBOM` {boolean} When `true`, the `TextDecoderStream` will include the
    byte order mark in the decoded result. When `false`, the byte order mark
    will be removed from the output. This option is only used when `encoding` is
    `'utf-8'`, `'utf-16be'` or `'utf-16le'`. **Default:** `false`.

Creates a new `TextDecoderStream` instance.

#### `textDecoderStream.encoding`
<!-- YAML
added: v16.6.0
-->

* Type: {string}

The encoding supported by the `TextDecoderStream` instance.

#### `textDecoderStream.fatal`
<!-- YAML
added: v16.6.0
-->

* Type: {boolean}

The value will be `true` if decoding errors result in a `TypeError` being
thrown.

#### `textDecoderStream.ignoreBOM`
<!-- YAML
added: v16.6.0
-->

* Type: {boolean}

The value will be `true` if the decoding result will include the byte order
mark.

#### `textDecoderStream.readable`
<!-- YAML
added: v16.6.0
-->

* Type: {ReadableStream}

#### `textDecoderStream.writable`
<!-- YAML
added: v16.6.0
-->

* Type: {WritableStream}

### Class: `CompressionStream`
<!-- YAML
added: REPLACEME
-->
#### `new CompressionStream(format)`
<!-- YAML
added: REPLACEME
-->

* `format` {string} One of either `'deflate'` or `'gzip'`.

#### `compressionStream.readable`
<!-- YAML
added: REPLACEME
-->

* Type: {ReadableStream}

#### `compressionStream.writable`
<!-- YAML
added: REPLACEME
-->

* Type: {WritableStream}

### Class: `DecompressionStream`
<!-- YAML
added: REPLACEME
-->

#### `new DecompressionStream(format)`
<!-- YAML
added: REPLACEME
-->

* `format` {string} One of either `'deflate'` or `'gzip'`.

#### `decompressionStream.readable`
<!-- YAML
added: REPLACEME
-->

* Type: {ReadableStream}

#### `deccompressionStream.writable`
<!-- YAML
added: REPLACEME
-->

* Type: {WritableStream}

[Streams]: stream.md
[WHATWG Streams Standard]: https://streams.spec.whatwg.org/
port1.onmessage = ({ data }) => {
  const { writable, readable } = data;
  // ...
};

port2.postMessage(stream, [stream]);
```

### Class: `TransformStreamDefaultController`
<!-- YAML
added: v16.5.0
-->

The `TransformStreamDefaultController` manages the internal state
of the `TransformStream`.

#### `transformStreamDefaultController.desiredSize`
<!-- YAML
added: v16.5.0
-->

* Type: {number}

The amount of data required to fill the readable side's queue.

#### `transformStreamDefaultController.enqueue([chunk])`
<!-- YAML
added: v16.5.0
-->

* `chunk` {any}

Appends a chunk of data to the readable side's queue.

#### `transformStreamDefaultController.error([reason])`
<!-- YAML
added: v16.5.0
-->

* `reason` {any}

Signals to both the readable and writable side that an error has occurred
while processing the transform data, causing both sides to be abruptly
closed.

#### `transformStreamDefaultController.terminate()`
<!-- YAML
added: v16.5.0
-->

Closes the readable side of the transport and causes the writable side
to be abruptly closed with an error.

### Class: `ByteLengthQueuingStrategy`
<!-- YAML
added: v16.5.0
-->

#### `new ByteLengthQueuingStrategy(options)`
<!-- YAML
added: v16.5.0
-->

* `options` {Object}
  * `highWaterMark` {number}

#### `byteLengthQueuingStrategy.highWaterMark`
<!-- YAML
added: v16.5.0
-->

* Type: {number}

#### `byteLengthQueuingStrategy.size`
<!-- YAML
added: v16.5.0
-->

* Type: {Function}
  * `chunk` {any}
  * Returns: {number}

### Class: `CountQueuingStrategy`
<!-- YAML
added: v16.5.0
-->

#### `new CountQueuingStrategy(options)`
<!-- YAML
added: v16.5.0
-->

* `options` {Object}
  * `highWaterMark` {number}

#### `countQueuingStrategy.highWaterMark`
<!-- YAML
added: v16.5.0
-->

* Type: {number}

#### `countQueuingStrategy.size`
<!-- YAML
added: v16.5.0
-->

* Type: {Function}
  * `chunk` {any}
  * Returns: {number}

### Class: `TextEncoderStream`
<!-- YAML
added: v16.6.0
-->

#### `new TextEncoderStream()`
<!-- YAML
added: v16.6.0
-->

Creates a new `TextEncoderStream` instance.

#### `textEncoderStream.encoding`
<!-- YAML
added: v16.6.0
-->

* Type: {string}

The encoding supported by the `TextEncoderStream` instance.

#### `textEncoderStream.readable`
<!-- YAML
added: v16.6.0
-->

* Type: {ReadableStream}

#### `textEncoderStream.writable`
<!-- YAML
added: v16.6.0
-->

* Type: {WritableStream}

### Class: `TextDecoderStream`
<!-- YAML
added: v16.6.0
-->

#### `new TextDecoderStream([encoding[, options]])`
<!-- YAML
added: v16.6.0
-->

* `encoding` {string} Identifies the `encoding` that this `TextDecoder` instance
  supports. **Default:** `'utf-8'`.
* `options` {Object}
  * `fatal` {boolean} `true` if decoding failures are fatal.
  * `ignoreBOM` {boolean} When `true`, the `TextDecoderStream` will include the
    byte order mark in the decoded result. When `false`, the byte order mark
    will be removed from the output. This option is only used when `encoding` is
    `'utf-8'`, `'utf-16be'` or `'utf-16le'`. **Default:** `false`.

Creates a new `TextDecoderStream` instance.

#### `textDecoderStream.encoding`
<!-- YAML
added: v16.6.0
-->

* Type: {string}

The encoding supported by the `TextDecoderStream` instance.

#### `textDecoderStream.fatal`
<!-- YAML
added: v16.6.0
-->

* Type: {boolean}

The value will be `true` if decoding errors result in a `TypeError` being
thrown.

#### `textDecoderStream.ignoreBOM`
<!-- YAML
added: v16.6.0
-->

* Type: {boolean}

The value will be `true` if the decoding result will include the byte order
mark.

#### `textDecoderStream.readable`
<!-- YAML
added: v16.6.0
-->

* Type: {ReadableStream}

#### `textDecoderStream.writable`
<!-- YAML
added: v16.6.0
-->

* Type: {WritableStream}

### Class: `CompressionStream`
<!-- YAML
added: REPLACEME
-->
#### `new CompressionStream(format)`
<!-- YAML
added: REPLACEME
-->

* `format` {string} One of either `'deflate'` or `'gzip'`.

#### `compressionStream.readable`
<!-- YAML
added: REPLACEME
-->

* Type: {ReadableStream}

#### `compressionStream.writable`
<!-- YAML
added: REPLACEME
-->

* Type: {WritableStream}

### Class: `DecompressionStream`
<!-- YAML
added: REPLACEME
-->

#### `new DecompressionStream(format)`
<!-- YAML
added: REPLACEME
-->

* `format` {string} One of either `'deflate'` or `'gzip'`.

#### `decompressionStream.readable`
<!-- YAML
added: REPLACEME
-->

* Type: {ReadableStream}

#### `deccompressionStream.writable`
<!-- YAML
added: REPLACEME
-->

* Type: {WritableStream}

[Streams]: stream.md
[WHATWG Streams Standard]: https://streams.spec.whatwg.org/