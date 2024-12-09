URL {
  href: 'http://localhost:3000/status?name=ryan',
  origin: 'http://localhost:3000',
  protocol: 'http:',
  username: '',
  password: '',
  host: 'localhost:3000',
  hostname: 'localhost',
  port: '3000',
  pathname: '/status',
  search: '?name=ryan',
  searchParams: URLSearchParams { 'name' => 'ryan' },
  hash: ''
}
```

## `http.METHODS`
<!-- YAML
added: v0.11.8
-->

* {string[]}

A list of the HTTP methods that are supported by the parser.

## `http.STATUS_CODES`
<!-- YAML
added: v0.1.22
-->

* {Object}

A collection of all the standard HTTP response status codes, and the
short description of each. For example, `http.STATUS_CODES[404] === 'Not
Found'`.

## `http.createServer([options][, requestListener])`
<!-- YAML
added: v0.1.13
changes:
  - version: REPLACEME
    pr-url: https://github.com/nodejs/node/pull/31448
    description: The `insecureHTTPParser` option is supported now.
  - version: v13.3.0
    pr-url: https://github.com/nodejs/node/pull/30570
    description: The `maxHeaderSize` option is supported now.
  - version: v9.6.0, v8.12.0
    pr-url: https://github.com/nodejs/node/pull/15752
    description: The `options` argument is supported now.
-->

* `options` {Object}
  * `IncomingMessage` {http.IncomingMessage} Specifies the `IncomingMessage`
    class to be used. Useful for extending the original `IncomingMessage`.
    **Default:** `IncomingMessage`.
  * `ServerResponse` {http.ServerResponse} Specifies the `ServerResponse` class
    to be used. Useful for extending the original `ServerResponse`. **Default:**
    `ServerResponse`.
  * `insecureHTTPParser` {boolean} Use an insecure HTTP parser that accepts
    invalid HTTP headers when `true`. Using the insecure parser should be
    avoided. See [`--insecure-http-parser`][] for more information.
    **Default:** `false`
  * `maxHeaderSize` {number} Optionally overrides the value of
    [`--max-http-header-size`][] for requests received by this server, i.e.
    the maximum length of request headers in bytes.
    **Default:** 8192 (8KB).
* `requestListener` {Function}

* Returns: {http.Server}

Returns a new instance of [`http.Server`][].

The `requestListener` is a function which is automatically
added to the [`'request'`][] event.

## `http.get(options[, callback])`
## `http.get(url[, options][, callback])`
<!-- YAML
added: v0.3.6
changes:
  - version: v10.9.0
    pr-url: https://github.com/nodejs/node/pull/21616
    description: The `url` parameter can now be passed along with a separate
                 `options` object.
  - version: v7.5.0
    pr-url: https://github.com/nodejs/node/pull/10638
    description: The `options` parameter can be a WHATWG `URL` object.
-->

* `url` {string | URL}
* `options` {Object} Accepts the same `options` as
  [`http.request()`][], with the `method` always set to `GET`.
  Properties that are inherited from the prototype are ignored.
* `callback` {Function}
* Returns: {http.ClientRequest}

Since most requests are GET requests without bodies, Node.js provides this
convenience method. The only difference between this method and
[`http.request()`][] is that it sets the method to GET and calls `req.end()`
automatically. The callback must take care to consume the response
data for reasons stated in [`http.ClientRequest`][] section.

The `callback` is invoked with a single argument that is an instance of
[`http.IncomingMessage`][].

JSON fetching example:

```js
http.get('http://nodejs.org/dist/index.json', (res) => {
  const { statusCode } = res;
  const contentType = res.headers['content-type'];

  let error;
  if (statusCode !== 200) {
    error = new Error('Request Failed.\n' +
                      `Status Code: ${statusCode}`);
  } else if (!/^application\/json/.test(contentType)) {
    error = new Error('Invalid content-type.\n' +
                      `Expected application/json but received ${contentType}`);
  }
  if (error) {
    console.error(error.message);
    // Consume response data to free up memory
    res.resume();
    return;
  }

  res.setEncoding('utf8');
  let rawData = '';
  res.on('data', (chunk) => { rawData += chunk; });
  res.on('end', () => {
    try {
      const parsedData = JSON.parse(rawData);
      console.log(parsedData);
    } catch (e) {
      console.error(e.message);
    }
  });
}).on('error', (e) => {
  console.error(`Got error: ${e.message}`);
});
```

## `http.globalAgent`
<!-- YAML
added: v0.5.9
-->

* {http.Agent}

Global instance of `Agent` which is used as the default for all HTTP client
requests.

## `http.maxHeaderSize`
<!-- YAML
added: v11.6.0
-->

* {number}

Read-only property specifying the maximum allowed size of HTTP headers in bytes.
Defaults to 8KB. Configurable using the [`--max-http-header-size`][] CLI option.

This can be overridden for servers and client requests by passing the
`maxHeaderSize` option.

## `http.request(options[, callback])`
## `http.request(url[, options][, callback])`
<!-- YAML
added: v0.3.6
changes:
  - version: REPLACEME
    pr-url: https://github.com/nodejs/node/pull/31448
    description: The `insecureHTTPParser` option is supported now.
  - version: v13.3.0
    pr-url: https://github.com/nodejs/node/pull/30570
    description: The `maxHeaderSize` option is supported now.
  - version: v10.9.0
    pr-url: https://github.com/nodejs/node/pull/21616
    description: The `url` parameter can now be passed along with a separate
                 `options` object.
  - version: v7.5.0
    pr-url: https://github.com/nodejs/node/pull/10638
    description: The `options` parameter can be a WHATWG `URL` object.
-->

* `url` {string | URL}
* `options` {Object}
  * `agent` {http.Agent | boolean} Controls [`Agent`][] behavior. Possible
    values:
    * `undefined` (default): use [`http.globalAgent`][] for this host and port.
    * `Agent` object: explicitly use the passed in `Agent`.
    * `false`: causes a new `Agent` with default values to be used.
  * `auth` {string} Basic authentication i.e. `'user:password'` to compute an
    Authorization header.
  * `createConnection` {Function} A function that produces a socket/stream to
    use for the request when the `agent` option is not used. This can be used to
    avoid creating a custom `Agent` class just to override the default
    `createConnection` function. See [`agent.createConnection()`][] for more
    details. Any [`Duplex`][] stream is a valid return value.
  * `defaultPort` {number} Default port for the protocol. **Default:**
    `agent.defaultPort` if an `Agent` is used, else `undefined`.
  * `family` {number} IP address family to use when resolving `host` or
    `hostname`. Valid values are `4` or `6`. When unspecified, both IP v4 and
    v6 will be used.
  * `headers` {Object} An object containing request headers.
  * `host` {string} A domain name or IP address of the server to issue the
    request to. **Default:** `'localhost'`.
  * `hostname` {string} Alias for `host`. To support [`url.parse()`][],
    `hostname` will be used if both `host` and `hostname` are specified.
  * `insecureHTTPParser` {boolean} Use an insecure HTTP parser that accepts
    invalid HTTP headers when `true`. Using the insecure parser should be
    avoided. See [`--insecure-http-parser`][] for more information.
    **Default:** `false`
  * `localAddress` {string} Local interface to bind for network connections.
  * `lookup` {Function} Custom lookup function. **Default:** [`dns.lookup()`][].
  * `maxHeaderSize` {number} Optionally overrides the value of
    [`--max-http-header-size`][] for requests received from the server, i.e.
    the maximum length of response headers in bytes.
    **Default:** 8192 (8KB).
  * `method` {string} A string specifying the HTTP request method. **Default:**
    `'GET'`.
  * `path` {string} Request path. Should include query string if any.
    E.G. `'/index.html?page=12'`. An exception is thrown when the request path
    contains illegal characters. Currently, only spaces are rejected but that
    may change in the future. **Default:** `'/'`.
  * `port` {number} Port of remote server. **Default:** `defaultPort` if set,
    else `80`.
  * `protocol` {string} Protocol to use. **Default:** `'http:'`.
  * `setHost` {boolean}: Specifies whether or not to automatically add the
    `Host` header. Defaults to `true`.
  * `socketPath` {string} Unix Domain Socket (cannot be used if one of `host`
     or `port` is specified, those specify a TCP Socket).
  * `timeout` {number}: A number specifying the socket timeout in milliseconds.
    This will set the timeout before the socket is connected.
* `callback` {Function}
* Returns: {http.ClientRequest}

Node.js maintains several connections per server to make HTTP requests.
This function allows one to transparently issue requests.

`url` can be a string or a [`URL`][] object. If `url` is a
string, it is automatically parsed with [`new URL()`][]. If it is a [`URL`][]
object, it will be automatically converted to an ordinary `options` object.

If both `url` and `options` are specified, the objects are merged, with the
`options` properties taking precedence.

The optional `callback` parameter will be added as a one-time listener for
the [`'response'`][] event.

`http.request()` returns an instance of the [`http.ClientRequest`][]
class. The `ClientRequest` instance is a writable stream. If one needs to
upload a file with a POST request, then write to the `ClientRequest` object.

```js
const postData = querystring.stringify({
  'msg': 'Hello World!'
});

const options = {
  hostname: 'www.google.com',
  port: 80,
  path: '/upload',
  method: 'POST',
  headers: {
    'Content-Type': 'application/x-www-form-urlencoded',
    'Content-Length': Buffer.byteLength(postData)
  }
};

const req = http.request(options, (res) => {
  console.log(`STATUS: ${res.statusCode}`);
  console.log(`HEADERS: ${JSON.stringify(res.headers)}`);
  res.setEncoding('utf8');
  res.on('data', (chunk) => {
    console.log(`BODY: ${chunk}`);
  });
  res.on('end', () => {
    console.log('No more data in response.');
  });
});

req.on('error', (e) => {
  console.error(`problem with request: ${e.message}`);
});

// Write data to request body
req.write(postData);
req.end();
```

In the example `req.end()` was called. With `http.request()` one
must always call `req.end()` to signify the end of the request -
even if there is no data being written to the request body.

If any error is encountered during the request (be that with DNS resolution,
TCP level errors, or actual HTTP parse errors) an `'error'` event is emitted
on the returned request object. As with all `'error'` events, if no listeners
are registered the error will be thrown.

There are a few special headers that should be noted.

* Sending a 'Connection: keep-alive' will notify Node.js that the connection to
  the server should be persisted until the next request.

* Sending a 'Content-Length' header will disable the default chunked encoding.

* Sending an 'Expect' header will immediately send the request headers.
  Usually, when sending 'Expect: 100-continue', both a timeout and a listener
  for the `'continue'` event should be set. See RFC 2616 Section 8.2.3 for more
  information.

* Sending an Authorization header will override using the `auth` option
  to compute basic authentication.

Example using a [`URL`][] as `options`:

```js
const options = new URL('http://abc:xyz@example.com');

const req = http.request(options, (res) => {
  // ...
});
```

In a successful request, the following events will be emitted in the following
order:

* `'socket'`
* `'response'`
  * `'data'` any number of times, on the `res` object
    (`'data'` will not be emitted at all if the response body is empty, for
    instance, in most redirects)
  * `'end'` on the `res` object
* `'close'`

In the case of a connection error, the following events will be emitted:

* `'socket'`
* `'error'`
* `'close'`

In the case of a premature connection close before the response is received,
the following events will be emitted in the following order:

* `'socket'`
* `'error'` with an error with message `'Error: socket hang up'` and code
  `'ECONNRESET'`
* `'close'`

In the case of a premature connection close after the response is received,
the following events will be emitted in the following order:

* `'socket'`
* `'response'`
  * `'data'` any number of times, on the `res` object
* (connection closed here)
* `'aborted'` on the `res` object
* `'close'`
* `'close'` on the `res` object

If `req.abort()` is called before the connection succeeds, the following events
will be emitted in the following order:

* `'socket'`
* (`req.abort()` called here)
* `'abort'`
* `'error'` with an error with message `'Error: socket hang up'` and code
  `'ECONNRESET'`
* `'close'`

If `req.abort()` is called after the response is received, the following events
will be emitted in the following order:

* `'socket'`
* `'response'`
  * `'data'` any number of times, on the `res` object
* (`req.abort()` called here)
* `'abort'`
* `'aborted'` on the `res` object
* `'close'`
* `'close'` on the `res` object

Setting the `timeout` option or using the `setTimeout()` function will
not abort the request or do anything besides add a `'timeout'` event.

[`--insecure-http-parser`]: cli.html#cli_insecure_http_parser
[`--max-http-header-size`]: cli.html#cli_max_http_header_size_size
[`'checkContinue'`]: #http_event_checkcontinue
[`'request'`]: #http_event_request
[`'response'`]: #http_event_response
[`'upgrade'`]: #http_event_upgrade
[`Agent`]: #http_class_http_agent
[`Duplex`]: stream.html#stream_class_stream_duplex
[`TypeError`]: errors.html#errors_class_typeerror
[`URL`]: url.html#url_the_whatwg_url_api
[`agent.createConnection()`]: #http_agent_createconnection_options_callback
[`agent.getName()`]: #http_agent_getname_options
[`destroy()`]: #http_agent_destroy
[`dns.lookup()`]: dns.html#dns_dns_lookup_hostname_options_callback
[`'finish'`]: #http_event_finish
[`getHeader(name)`]: #http_request_getheader_name
[`http.Agent`]: #http_class_http_agent
[`http.ClientRequest`]: #http_class_http_clientrequest
[`http.IncomingMessage`]: #http_class_http_incomingmessage
[`http.Server`]: #http_class_http_server
[`http.get()`]: #http_http_get_options_callback
[`http.globalAgent`]: #http_http_globalagent
[`http.request()`]: #http_http_request_options_callback
[`message.headers`]: #http_message_headers
[`net.Server.close()`]: net.html#net_server_close_callback
[`net.Server`]: net.html#net_class_net_server
[`net.Socket`]: net.html#net_class_net_socket
[`net.createConnection()`]: net.html#net_net_createconnection_options_connectlistener
[`new URL()`]: url.html#url_constructor_new_url_input_base
[`removeHeader(name)`]: #http_request_removeheader_name
[`request.end()`]: #http_request_end_data_encoding_callback
[`request.flushHeaders()`]: #http_request_flushheaders
[`request.getHeader()`]: #http_request_getheader_name
[`request.setHeader()`]: #http_request_setheader_name_value
[`request.setTimeout()`]: #http_request_settimeout_timeout_callback
[`request.socket.getPeerCertificate()`]: tls.html#tls_tlssocket_getpeercertificate_detailed
[`request.socket`]: #http_request_socket
[`request.writableFinished`]: #http_request_writablefinished
[`request.writableEnded`]: #http_request_writableended
[`request.write(data, encoding)`]: #http_request_write_chunk_encoding_callback
[`response.end()`]: #http_response_end_data_encoding_callback
[`response.getHeader()`]: #http_response_getheader_name
[`response.setHeader()`]: #http_response_setheader_name_value
[`response.socket`]: #http_response_socket
[`response.writableFinished`]: #http_response_writablefinished
[`response.writableEnded`]: #http_response_writableended
[`response.write()`]: #http_response_write_chunk_encoding_callback
[`response.write(data, encoding)`]: #http_response_write_chunk_encoding_callback
[`response.writeContinue()`]: #http_response_writecontinue
[`response.writeHead()`]: #http_response_writehead_statuscode_statusmessage_headers
[`server.listen()`]: net.html#net_server_listen
[`server.timeout`]: #http_server_timeout
[`setHeader(name, value)`]: #http_request_setheader_name_value
[`socket.connect()`]: net.html#net_socket_connect_options_connectlistener
[`socket.setKeepAlive()`]: net.html#net_socket_setkeepalive_enable_initialdelay
[`socket.setNoDelay()`]: net.html#net_socket_setnodelay_nodelay
[`socket.setTimeout()`]: net.html#net_socket_settimeout_timeout_callback
[`socket.unref()`]: net.html#net_socket_unref
[`url.parse()`]: url.html#url_url_parse_urlstring_parsequerystring_slashesdenotehost
[`HPE_HEADER_OVERFLOW`]: errors.html#errors_hpe_header_overflow
[`writable.cork()`]: stream.html#stream_writable_cork
[`writable.uncork()`]: stream.html#stream_writable_uncork
}).on('error', (e) => {
  console.error(`Got error: ${e.message}`);
});
```

## `http.globalAgent`
<!-- YAML
added: v0.5.9
-->

* {http.Agent}

Global instance of `Agent` which is used as the default for all HTTP client
requests.

## `http.maxHeaderSize`
<!-- YAML
added: v11.6.0
-->

* {number}

Read-only property specifying the maximum allowed size of HTTP headers in bytes.
Defaults to 8KB. Configurable using the [`--max-http-header-size`][] CLI option.

This can be overridden for servers and client requests by passing the
`maxHeaderSize` option.

## `http.request(options[, callback])`
## `http.request(url[, options][, callback])`
<!-- YAML
added: v0.3.6
changes:
  - version: REPLACEME
    pr-url: https://github.com/nodejs/node/pull/31448
    description: The `insecureHTTPParser` option is supported now.
  - version: v13.3.0
    pr-url: https://github.com/nodejs/node/pull/30570
    description: The `maxHeaderSize` option is supported now.
  - version: v10.9.0
    pr-url: https://github.com/nodejs/node/pull/21616
    description: The `url` parameter can now be passed along with a separate
                 `options` object.
  - version: v7.5.0
    pr-url: https://github.com/nodejs/node/pull/10638
    description: The `options` parameter can be a WHATWG `URL` object.
-->

* `url` {string | URL}
* `options` {Object}
  * `agent` {http.Agent | boolean} Controls [`Agent`][] behavior. Possible
    values:
    * `undefined` (default): use [`http.globalAgent`][] for this host and port.
    * `Agent` object: explicitly use the passed in `Agent`.
    * `false`: causes a new `Agent` with default values to be used.
  * `auth` {string} Basic authentication i.e. `'user:password'` to compute an
    Authorization header.
  * `createConnection` {Function} A function that produces a socket/stream to
    use for the request when the `agent` option is not used. This can be used to
    avoid creating a custom `Agent` class just to override the default
    `createConnection` function. See [`agent.createConnection()`][] for more
    details. Any [`Duplex`][] stream is a valid return value.
  * `defaultPort` {number} Default port for the protocol. **Default:**
    `agent.defaultPort` if an `Agent` is used, else `undefined`.
  * `family` {number} IP address family to use when resolving `host` or
    `hostname`. Valid values are `4` or `6`. When unspecified, both IP v4 and
    v6 will be used.
  * `headers` {Object} An object containing request headers.
  * `host` {string} A domain name or IP address of the server to issue the
    request to. **Default:** `'localhost'`.
  * `hostname` {string} Alias for `host`. To support [`url.parse()`][],
    `hostname` will be used if both `host` and `hostname` are specified.
  * `insecureHTTPParser` {boolean} Use an insecure HTTP parser that accepts
    invalid HTTP headers when `true`. Using the insecure parser should be
    avoided. See [`--insecure-http-parser`][] for more information.
    **Default:** `false`
  * `localAddress` {string} Local interface to bind for network connections.
  * `lookup` {Function} Custom lookup function. **Default:** [`dns.lookup()`][].
  * `maxHeaderSize` {number} Optionally overrides the value of
    [`--max-http-header-size`][] for requests received from the server, i.e.
    the maximum length of response headers in bytes.
    **Default:** 8192 (8KB).
  * `method` {string} A string specifying the HTTP request method. **Default:**
    `'GET'`.
  * `path` {string} Request path. Should include query string if any.
    E.G. `'/index.html?page=12'`. An exception is thrown when the request path
    contains illegal characters. Currently, only spaces are rejected but that
    may change in the future. **Default:** `'/'`.
  * `port` {number} Port of remote server. **Default:** `defaultPort` if set,
    else `80`.
  * `protocol` {string} Protocol to use. **Default:** `'http:'`.
  * `setHost` {boolean}: Specifies whether or not to automatically add the
    `Host` header. Defaults to `true`.
  * `socketPath` {string} Unix Domain Socket (cannot be used if one of `host`
     or `port` is specified, those specify a TCP Socket).
  * `timeout` {number}: A number specifying the socket timeout in milliseconds.
    This will set the timeout before the socket is connected.
* `callback` {Function}
* Returns: {http.ClientRequest}

Node.js maintains several connections per server to make HTTP requests.
This function allows one to transparently issue requests.

`url` can be a string or a [`URL`][] object. If `url` is a
string, it is automatically parsed with [`new URL()`][]. If it is a [`URL`][]
object, it will be automatically converted to an ordinary `options` object.

If both `url` and `options` are specified, the objects are merged, with the
`options` properties taking precedence.

The optional `callback` parameter will be added as a one-time listener for
the [`'response'`][] event.

`http.request()` returns an instance of the [`http.ClientRequest`][]
class. The `ClientRequest` instance is a writable stream. If one needs to
upload a file with a POST request, then write to the `ClientRequest` object.

```js
const postData = querystring.stringify({
  'msg': 'Hello World!'
});

const options = {
  hostname: 'www.google.com',
  port: 80,
  path: '/upload',
  method: 'POST',
  headers: {
    'Content-Type': 'application/x-www-form-urlencoded',
    'Content-Length': Buffer.byteLength(postData)
  }
};

const req = http.request(options, (res) => {
  console.log(`STATUS: ${res.statusCode}`);
  console.log(`HEADERS: ${JSON.stringify(res.headers)}`);
  res.setEncoding('utf8');
  res.on('data', (chunk) => {
    console.log(`BODY: ${chunk}`);
  });
  res.on('end', () => {
    console.log('No more data in response.');
  });
});

req.on('error', (e) => {
  console.error(`problem with request: ${e.message}`);
});

// Write data to request body
req.write(postData);
req.end();
```

In the example `req.end()` was called. With `http.request()` one
must always call `req.end()` to signify the end of the request -
even if there is no data being written to the request body.

If any error is encountered during the request (be that with DNS resolution,
TCP level errors, or actual HTTP parse errors) an `'error'` event is emitted
on the returned request object. As with all `'error'` events, if no listeners
are registered the error will be thrown.

There are a few special headers that should be noted.

* Sending a 'Connection: keep-alive' will notify Node.js that the connection to
  the server should be persisted until the next request.

* Sending a 'Content-Length' header will disable the default chunked encoding.

* Sending an 'Expect' header will immediately send the request headers.
  Usually, when sending 'Expect: 100-continue', both a timeout and a listener
  for the `'continue'` event should be set. See RFC 2616 Section 8.2.3 for more
  information.

* Sending an Authorization header will override using the `auth` option
  to compute basic authentication.

Example using a [`URL`][] as `options`:

```js
const options = new URL('http://abc:xyz@example.com');

const req = http.request(options, (res) => {
  // ...
});
```

In a successful request, the following events will be emitted in the following
order:

* `'socket'`
* `'response'`
  * `'data'` any number of times, on the `res` object
    (`'data'` will not be emitted at all if the response body is empty, for
    instance, in most redirects)
  * `'end'` on the `res` object
* `'close'`

In the case of a connection error, the following events will be emitted:

* `'socket'`
* `'error'`
* `'close'`

In the case of a premature connection close before the response is received,
the following events will be emitted in the following order:

* `'socket'`
* `'error'` with an error with message `'Error: socket hang up'` and code
  `'ECONNRESET'`
* `'close'`

In the case of a premature connection close after the response is received,
the following events will be emitted in the following order:

* `'socket'`
* `'response'`
  * `'data'` any number of times, on the `res` object
* (connection closed here)
* `'aborted'` on the `res` object
* `'close'`
* `'close'` on the `res` object

If `req.abort()` is called before the connection succeeds, the following events
will be emitted in the following order:

* `'socket'`
* (`req.abort()` called here)
* `'abort'`
* `'error'` with an error with message `'Error: socket hang up'` and code
  `'ECONNRESET'`
* `'close'`

If `req.abort()` is called after the response is received, the following events
will be emitted in the following order:

* `'socket'`
* `'response'`
  * `'data'` any number of times, on the `res` object
* (`req.abort()` called here)
* `'abort'`
* `'aborted'` on the `res` object
* `'close'`
* `'close'` on the `res` object

Setting the `timeout` option or using the `setTimeout()` function will
not abort the request or do anything besides add a `'timeout'` event.

[`--insecure-http-parser`]: cli.html#cli_insecure_http_parser
[`--max-http-header-size`]: cli.html#cli_max_http_header_size_size
[`'checkContinue'`]: #http_event_checkcontinue
[`'request'`]: #http_event_request
[`'response'`]: #http_event_response
[`'upgrade'`]: #http_event_upgrade
[`Agent`]: #http_class_http_agent
[`Duplex`]: stream.html#stream_class_stream_duplex
[`TypeError`]: errors.html#errors_class_typeerror
[`URL`]: url.html#url_the_whatwg_url_api
[`agent.createConnection()`]: #http_agent_createconnection_options_callback
[`agent.getName()`]: #http_agent_getname_options
[`destroy()`]: #http_agent_destroy
[`dns.lookup()`]: dns.html#dns_dns_lookup_hostname_options_callback
[`'finish'`]: #http_event_finish
[`getHeader(name)`]: #http_request_getheader_name
[`http.Agent`]: #http_class_http_agent
[`http.ClientRequest`]: #http_class_http_clientrequest
[`http.IncomingMessage`]: #http_class_http_incomingmessage
[`http.Server`]: #http_class_http_server
[`http.get()`]: #http_http_get_options_callback
[`http.globalAgent`]: #http_http_globalagent
[`http.request()`]: #http_http_request_options_callback
[`message.headers`]: #http_message_headers
[`net.Server.close()`]: net.html#net_server_close_callback
[`net.Server`]: net.html#net_class_net_server
[`net.Socket`]: net.html#net_class_net_socket
[`net.createConnection()`]: net.html#net_net_createconnection_options_connectlistener
[`new URL()`]: url.html#url_constructor_new_url_input_base
[`removeHeader(name)`]: #http_request_removeheader_name
[`request.end()`]: #http_request_end_data_encoding_callback
[`request.flushHeaders()`]: #http_request_flushheaders
[`request.getHeader()`]: #http_request_getheader_name
[`request.setHeader()`]: #http_request_setheader_name_value
[`request.setTimeout()`]: #http_request_settimeout_timeout_callback
[`request.socket.getPeerCertificate()`]: tls.html#tls_tlssocket_getpeercertificate_detailed
[`request.socket`]: #http_request_socket
[`request.writableFinished`]: #http_request_writablefinished
[`request.writableEnded`]: #http_request_writableended
[`request.write(data, encoding)`]: #http_request_write_chunk_encoding_callback
[`response.end()`]: #http_response_end_data_encoding_callback
[`response.getHeader()`]: #http_response_getheader_name
[`response.setHeader()`]: #http_response_setheader_name_value
[`response.socket`]: #http_response_socket
[`response.writableFinished`]: #http_response_writablefinished
[`response.writableEnded`]: #http_response_writableended
[`response.write()`]: #http_response_write_chunk_encoding_callback
[`response.write(data, encoding)`]: #http_response_write_chunk_encoding_callback
[`response.writeContinue()`]: #http_response_writecontinue
[`response.writeHead()`]: #http_response_writehead_statuscode_statusmessage_headers
[`server.listen()`]: net.html#net_server_listen
[`server.timeout`]: #http_server_timeout
[`setHeader(name, value)`]: #http_request_setheader_name_value
[`socket.connect()`]: net.html#net_socket_connect_options_connectlistener
[`socket.setKeepAlive()`]: net.html#net_socket_setkeepalive_enable_initialdelay
[`socket.setNoDelay()`]: net.html#net_socket_setnodelay_nodelay
[`socket.setTimeout()`]: net.html#net_socket_settimeout_timeout_callback
[`socket.unref()`]: net.html#net_socket_unref
[`url.parse()`]: url.html#url_url_parse_urlstring_parsequerystring_slashesdenotehost
[`HPE_HEADER_OVERFLOW`]: errors.html#errors_hpe_header_overflow
[`writable.cork()`]: stream.html#stream_writable_cork
[`writable.uncork()`]: stream.html#stream_writable_uncork