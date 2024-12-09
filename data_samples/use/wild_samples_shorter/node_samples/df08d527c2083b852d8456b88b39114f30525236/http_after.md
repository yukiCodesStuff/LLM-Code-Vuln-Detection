
Limits maximum incoming headers count. If set to 0, no limit will be applied.

### `server.requestTimeout`
<!-- YAML
added: REPLACEME
-->

* {number} **Default:** `0`

Sets the timeout value in milliseconds for receiving the entire request from
the client.

If the timeout expires, the server responds with status 408 without
forwarding the request to the request listener and then closes the connection.

It must be set to a non-zero value (e.g. 120 seconds) to proctect against
potential Denial-of-Service attacks in case the server is deployed without a
reverse proxy in front.

### `server.setTimeout([msecs][, callback])`
<!-- YAML
added: v0.9.12
changes: