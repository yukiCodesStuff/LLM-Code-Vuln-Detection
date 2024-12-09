The `'unknownProtocol'` event is emitted when a connecting client fails to
negotiate an allowed protocol (i.e. HTTP/2 or HTTP/1.1). The event handler
receives the socket for handling. If no listener is registered for this event,
the connection is terminated. A timeout may be specified using the
`'unknownProtocolTimeout'` option passed to [`http2.createSecureServer()`][].
See the [Compatibility API][].

#### `server.close([callback])`
<!-- YAML
added: v8.4.0
<!-- YAML
added: v8.4.0
changes:
  - version: REPLACEME
    pr-url: https://github.com/nodejs-private/node-private/pull/246
    description: Added `unknownProtocolTimeout` option with a default of 10000.
  - version:
     - v14.4.0
     - v12.18.0
     - v10.21.0
    `Http2ServerResponse` class to use.
    Useful for extending the original `Http2ServerResponse`.
    **Default:** `Http2ServerResponse`.
  * `unknownProtocolTimeout` {number} Specifies a timeout in milliseconds that
    a server should wait when an [`'unknownProtocol'`][] is emitted. If the
    socket has not been destroyed by that time the server will destroy it.
    **Default:** `10000`.
  * ...: Any [`net.createServer()`][] option can be provided.
* `onRequestHandler` {Function} See [Compatibility API][]
* Returns: {Http2Server}

<!-- YAML
added: v8.4.0
changes:
  - version: REPLACEME
    pr-url: https://github.com/nodejs-private/node-private/pull/246
    description: Added `unknownProtocolTimeout` option with a default of 10000.
  - version:
     - v14.4.0
     - v12.18.0
     - v10.21.0
    servers, the identity options (`pfx` or `key`/`cert`) are usually required.
  * `origins` {string[]} An array of origin strings to send within an `ORIGIN`
    frame immediately following creation of a new server `Http2Session`.
  * `unknownProtocolTimeout` {number} Specifies a timeout in milliseconds that
    a server should wait when an [`'unknownProtocol'`][] event is emitted. If
    the socket has not been destroyed by that time the server will destroy it.
    **Default:** `10000`.
* `onRequestHandler` {Function} See [Compatibility API][]
* Returns: {Http2SecureServer}

Returns a `tls.Server` instance that creates and manages `Http2Session`
<!-- YAML
added: v8.4.0
changes:
  - version: REPLACEME
    pr-url: https://github.com/nodejs-private/node-private/pull/246
    description: Added `unknownProtocolTimeout` option with a default of 10000.
  - version:
     - v14.4.0
     - v12.18.0
     - v10.21.0
    instance passed to `connect` and the `options` object, and returns any
    [`Duplex`][] stream that is to be used as the connection for this session.
  * ...: Any [`net.connect()`][] or [`tls.connect()`][] options can be provided.
  * `unknownProtocolTimeout` {number} Specifies a timeout in milliseconds that
    a server should wait when an [`'unknownProtocol'`][] event is emitted. If
    the socket has not been destroyed by that time the server will destroy it.
    **Default:** `10000`.
* `listener` {Function} Will be registered as a one-time listener of the
  [`'connect'`][] event.
* Returns: {ClientHttp2Session}
