
#### http2session.ping([payload, ]callback)
<!-- YAML
added: REPLACEME
-->

* `payload` {Buffer|TypedArray|DataView} Optional ping payload.
* `callback` {Function}
<!-- YAML
added: v8.4.0
changes:
  - version: REPLACEME
    pr-url: https://github.com/nodejs/node/pull/17105
    description: Added the `maxOutstandingPings` option with a default limit of
                 10.
  - version: v9.1.0
    pr-url: https://github.com/nodejs/node/pull/16676
    description: Added the `maxHeaderListPairs` option with a default limit of
                 128 header pairs.
-->
<!-- YAML
added: v8.4.0
changes:
  - version: REPLACEME
    pr-url: https://github.com/nodejs/node/pull/17105
    description: Added the `maxOutstandingPings` option with a default limit of
                 10.
  - version: v9.1.0
    pr-url: https://github.com/nodejs/node/pull/16676
    description: Added the `maxHeaderListPairs` option with a default limit of
                 128 header pairs.
-->
<!-- YAML
added: v8.4.0
changes:
  - version: REPLACEME
    pr-url: https://github.com/nodejs/node/pull/17105
    description: Added the `maxOutstandingPings` option with a default limit of
                 10.
  - version: v9.1.0
    pr-url: https://github.com/nodejs/node/pull/16676
    description: Added the `maxHeaderListPairs` option with a default limit of
                 128 header pairs.
-->
<!-- YAML
added: v8.4.0
changes:
  - version: v9.1.0
    pr-url: https://github.com/nodejs/node/pull/16676
    description: The `maxHeaderListSize` setting is now strictly enforced.
-->
The `http2.getDefaultSettings()`, `http2.getPackedSettings()`,