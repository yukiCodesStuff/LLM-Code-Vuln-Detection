https.globalAgent.on('keylog', (line, tlsSocket) => {
  fs.appendFileSync('/tmp/ssl-keys.log', line, { mode: 0o600 });
});
```

## Class: `https.Server`
<!-- YAML
added: v0.3.4
-->

* Extends: {tls.Server}

See [`http.Server`][] for more information.

### `server.close([callback])`
<!-- YAML
added: v0.1.90
-->

* `callback` {Function}
* Returns: {https.Server}

See [`server.close()`][`http.close()`] from the HTTP module for details.

### `server.headersTimeout`
<!-- YAML
added: v11.3.0
-->

* {number} **Default:** `60000`

See [`http.Server#headersTimeout`][].

### `server.listen()`

Starts the HTTPS server listening for encrypted connections.
This method is identical to [`server.listen()`][] from [`net.Server`][].

### `server.maxHeadersCount`

* {number} **Default:** `2000`

See [`http.Server#maxHeadersCount`][].

### `server.requestTimeout`
<!-- YAML
added: v14.11.0
-->

* {number} **Default:** `0`

See [`http.Server#requestTimeout`][].

### `server.setTimeout([msecs][, callback])`
<!-- YAML
added: v0.11.2
-->

* `msecs` {number} **Default:** `120000` (2 minutes)
* `callback` {Function}
* Returns: {https.Server}

See [`http.Server#setTimeout()`][].

### `server.timeout`
<!-- YAML
added: v0.11.2
changes:
  - version: v13.0.0
    pr-url: https://github.com/nodejs/node/pull/27558
    description: The default timeout changed from 120s to 0 (no timeout).
-->

* {number} **Default:** 0 (no timeout)

See [`http.Server#timeout`][].

### `server.keepAliveTimeout`
<!-- YAML
added: v8.0.0
-->

* {number} **Default:** `5000` (5 seconds)

See [`http.Server#keepAliveTimeout`][].

## `https.createServer([options][, requestListener])`
<!-- YAML
added: v0.3.4
-->

* `options` {Object} Accepts `options` from [`tls.createServer()`][],
 [`tls.createSecureContext()`][] and [`http.createServer()`][].
* `requestListener` {Function} A listener to be added to the `'request'` event.
* Returns: {https.Server}

```js
// curl -k https://localhost:8000/
const https = require('https');
const fs = require('fs');

const options = {
  key: fs.readFileSync('test/fixtures/keys/agent2-key.pem'),
  cert: fs.readFileSync('test/fixtures/keys/agent2-cert.pem')
};

https.createServer(options, (req, res) => {
  res.writeHead(200);
  res.end('hello world\n');
}).listen(8000);
```

Or

```js
const https = require('https');
const fs = require('fs');

const options = {
  pfx: fs.readFileSync('test/fixtures/test_cert.pfx'),
  passphrase: 'sample'
};

https.createServer(options, (req, res) => {
  res.writeHead(200);
  res.end('hello world\n');
}).listen(8000);
```

## `https.get(options[, callback])`
## `https.get(url[, options][, callback])`
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