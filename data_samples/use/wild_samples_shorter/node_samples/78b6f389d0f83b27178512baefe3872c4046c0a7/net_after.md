
## Class: `net.SocketAddress`
<!-- YAML
added: v15.14.0
-->
### `new net.SocketAddress([options])`
<!-- YAML
added: v15.14.0
-->

* `options` {Object}
  * `address` {string} The network address as either an IPv4 or IPv6 string.

### `socketaddress.address`
<!-- YAML
added: v15.14.0
-->

* Type {string}

### `socketaddress.family`
<!-- YAML
added: v15.14.0
-->

* Type {string} Either `'ipv4'` or `'ipv6'`.

### `socketaddress.flowlabel`
<!-- YAML
added: v15.14.0
-->

* Type {number}

### `socketaddress.port`
<!-- YAML
added: v15.14.0
-->

* Type {number}

<!-- YAML
added: v0.3.4
changes:
  - version: v15.14.0
    pr-url: https://github.com/nodejs/node/pull/37735
    description: AbortSignal support was added.
-->
