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
added: REPLACEME
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
req.on('error', (e) => {
  console.error(e.message);
});
req.end();
```

Outputs for example:

```text
Subject Common Name: github.com
  Certificate SHA256 fingerprint: 25:FE:39:32:D9:63:8C:8A:FC:A1:9A:29:87:D8:3E:4C:1D:98:DB:71:E4:1A:48:03:98:EA:22:6A:BD:8B:93:16
  Public key ping-sha256: pL1+qb9HTMRZJmuC/bB/ZI9d302BYrrqiVuRyW+DGrU=
Subject Common Name: DigiCert SHA2 Extended Validation Server CA
  Certificate SHA256 fingerprint: 40:3E:06:2A:26:53:05:91:13:28:5B:AF:80:A0:D4:AE:42:2C:84:8C:9F:78:FA:D0:1F:C9:4B:C5:B8:7F:EF:1A
  Public key ping-sha256: RRM1dGqnDFsCJXBTHky16vi1obOlCgFFn/yOhI/y+ho=
Subject Common Name: DigiCert High Assurance EV Root CA
  Certificate SHA256 fingerprint: 74:31:E5:F4:C3:C1:CE:46:90:77:4F:0B:61:E0:54:40:88:3B:A9:A0:1E:D0:0B:A6:AB:D7:80:6E:D3:B1:18:CF
  Public key ping-sha256: WoiWRyIOVNa9ihaBciRSC7XHjliYS9VwUGOIud4PB18=
All OK. Server matched our pinned cert or public key
statusCode: 200
headers: max-age=0; pin-sha256="WoiWRyIOVNa9ihaBciRSC7XHjliYS9VwUGOIud4PB18="; pin-sha256="RRM1dGqnDFsCJXBTHky16vi1obOlCgFFn/yOhI/y+ho="; pin-sha256="k2v657xBsOVe1PQRwOsHsw3bsGT2VzIqz5K+59sNQws="; pin-sha256="K87oWBWM9UZfyddvDfoxL+8lpNyoUB2ptGtn0fv6G2Q="; pin-sha256="IQBnNBEiFuhj+8x6X8XLgh01V9Ic5/V3IRQLNFFc7v4="; pin-sha256="iie1VXtL7HzAMF+/PVPR9xzT80kQxdZeJ+zduCB3uj0="; pin-sha256="LvRiGEjRqfzurezaWuj8Wie2gyHMrW5Q06LspMnox7A="; includeSubDomains
```

[`Agent`]: #https_class_https_agent
[`URL`]: url.html#url_the_whatwg_url_api
[`http.Agent`]: http.html#http_class_http_agent
[`http.Agent(options)`]: http.html#http_new_agent_options
[`http.Server#headersTimeout`]: http.html#http_server_headerstimeout
[`http.Server#keepAliveTimeout`]: http.html#http_server_keepalivetimeout
[`http.Server#maxHeadersCount`]: http.html#http_server_maxheaderscount
[`http.Server#requestTimeout`]: http.html#http_server_requesttimeout
[`http.Server#setTimeout()`]: http.html#http_server_settimeout_msecs_callback
[`http.Server#timeout`]: http.html#http_server_timeout
[`http.Server`]: http.html#http_class_http_server
[`http.close()`]: http.html#http_server_close_callback
[`http.createServer()`]: http.html#http_http_createserver_options_requestlistener
[`http.get()`]: http.html#http_http_get_options_callback
[`http.request()`]: http.html#http_http_request_options_callback
[`https.Agent`]: #https_class_https_agent
[`https.request()`]: #https_https_request_options_callback
[`net.Server`]: net.html#net_class_net_server
[`new URL()`]: url.html#url_new_url_input_base
[`server.listen()`]: net.html#net_server_listen
[`tls.connect()`]: tls.html#tls_tls_connect_options_callback
[`tls.createSecureContext()`]: tls.html#tls_tls_createsecurecontext_options
[`tls.createServer()`]: tls.html#tls_tls_createserver_options_secureconnectionlistener
[`Session Resumption`]: tls.html#tls_session_resumption
[sni wiki]: https://en.wikipedia.org/wiki/Server_Name_Indication