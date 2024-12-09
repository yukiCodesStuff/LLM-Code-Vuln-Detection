      "data": {
        "version": 3,
        "sources": [
          "file:///absolute/path/to/original.js"
        ],
        "names": [
          "Foo",
          "console",
          "info"
        ],
        "mappings": "MAAMA,IACJC,YAAaC",
        "sourceRoot": "./"
      },
      "lineLengths": [
        13,
        62,
        38,
        27
      ]
    }
  }
}
```

### `OPENSSL_CONF=file`

<!-- YAML
added: v6.11.0
-->

Load an OpenSSL configuration file on startup. Among other uses, this can be
used to enable FIPS-compliant crypto if Node.js is built with
`./configure --openssl-fips`.

If the [`--openssl-config`][] command-line option is used, the environment
variable is ignored.

### `SSL_CERT_DIR=dir`

<!-- YAML
added: v7.7.0
-->

If `--use-openssl-ca` is enabled, this overrides and sets OpenSSL's directory
containing trusted certificates.

Be aware that unless the child environment is explicitly set, this environment
variable will be inherited by any child processes, and if they use OpenSSL, it
may cause them to trust the same CAs as node.

### `SSL_CERT_FILE=file`

<!-- YAML
added: v7.7.0
-->

If `--use-openssl-ca` is enabled, this overrides and sets OpenSSL's file
containing trusted certificates.

Be aware that unless the child environment is explicitly set, this environment
variable will be inherited by any child processes, and if they use OpenSSL, it
may cause them to trust the same CAs as node.

### `TZ`

<!-- YAML
added: v0.0.1
changes:
  - version:
     - v16.2.0
    pr-url: https://github.com/nodejs/node/pull/38642
    description:
      Changing the TZ variable using process.env.TZ = changes the timezone
      on Windows as well.
  - version:
     - v13.0.0
    pr-url: https://github.com/nodejs/node/pull/20026
    description:
      Changing the TZ variable using process.env.TZ = changes the timezone
      on POSIX systems.
-->

The `TZ` environment variable is used to specify the timezone configuration.

While Node.js does not support all of the various [ways that `TZ` is handled in
other environments][], it does support basic [timezone IDs][] (such as
`'Etc/UTC'`, `'Europe/Paris'`, or `'America/New_York'`).
It may support a few other abbreviations or aliases, but these are strongly
discouraged and not guaranteed.

```console
$ TZ=Europe/Dublin node -pe "new Date().toString()"
Wed May 12 2021 20:30:48 GMT+0100 (Irish Standard Time)
```

### `UV_THREADPOOL_SIZE=size`

Set the number of threads used in libuv's threadpool to `size` threads.

Asynchronous system APIs are used by Node.js whenever possible, but where they
do not exist, libuv's threadpool is used to create asynchronous node APIs based
on synchronous system APIs. Node.js APIs that use the threadpool are:

* all `fs` APIs, other than the file watcher APIs and those that are explicitly
  synchronous
* asynchronous crypto APIs such as `crypto.pbkdf2()`, `crypto.scrypt()`,
  `crypto.randomBytes()`, `crypto.randomFill()`, `crypto.generateKeyPair()`
* `dns.lookup()`
* all `zlib` APIs, other than those that are explicitly synchronous

Because libuv's threadpool has a fixed size, it means that if for whatever
reason any of these APIs takes a long time, other (seemingly unrelated) APIs
that run in libuv's threadpool will experience degraded performance. In order to
mitigate this issue, one potential solution is to increase the size of libuv's
threadpool by setting the `'UV_THREADPOOL_SIZE'` environment variable to a value
greater than `4` (its current default value). For more information, see the
[libuv threadpool documentation][].

### `UV_USE_IO_URING=value`

Enable or disable libuv's use of `io_uring` on supported platforms.

On supported platforms, `io_uring` can significantly improve the performance of
various asynchronous I/O operations.

`io_uring` is disabled by default due to security concerns. When `io_uring`
is enabled, applications must not change the user identity of the process at
runtime, neither through JavaScript functions such as [`process.setuid()`][] nor
through native addons that can invoke system functions such as [`setuid(2)`][].

This environment variable is implemented by a dependency of Node.js and may be
removed in future versions of Node.js. No stability guarantees are provided for
the behavior of this environment variable.

## Useful V8 options

V8 has its own set of CLI options. Any V8 CLI option that is provided to `node`
will be passed on to V8 to handle. V8's options have _no stability guarantee_.
The V8 team themselves don't consider them to be part of their formal API,
and reserve the right to change them at any time. Likewise, they are not
covered by the Node.js stability guarantees. Many of the V8
options are of interest only to V8 developers. Despite this, there is a small
set of V8 options that are widely applicable to Node.js, and they are
documented here:

### `--max-old-space-size=SIZE` (in megabytes)

Sets the max memory size of V8's old memory section. As memory
consumption approaches the limit, V8 will spend more time on
garbage collection in an effort to free unused memory.

On a machine with 2 GiB of memory, consider setting this to
1536 (1.5 GiB) to leave some memory for other uses and avoid swapping.

```bash
node --max-old-space-size=1536 index.js
```

### `--max-semi-space-size=SIZE` (in megabytes)

Sets the maximum [semi-space][] size for V8's [scavenge garbage collector][] in
MiB (megabytes).
Increasing the max size of a semi-space may improve throughput for Node.js at
the cost of more memory consumption.

Since the young generation size of the V8 heap is three times (see
[`YoungGenerationSizeFromSemiSpaceSize`][] in V8) the size of the semi-space,
an increase of 1 MiB to semi-space applies to each of the three individual
semi-spaces and causes the heap size to increase by 3 MiB. The throughput
improvement depends on your workload (see [#42511][]).

The default value is 16 MiB for 64-bit systems and 8 MiB for 32-bit systems. To
get the best configuration for your application, you should try different
max-semi-space-size values when running benchmarks for your application.

For example, benchmark on a 64-bit systems:

```bash
for MiB in 16 32 64 128; do
    node --max-semi-space-size=$MiB index.js
done
```

[#42511]: https://github.com/nodejs/node/issues/42511
[Chrome DevTools Protocol]: https://chromedevtools.github.io/devtools-protocol/
[CommonJS]: modules.md
[CommonJS module]: modules.md
[CustomEvent Web API]: https://dom.spec.whatwg.org/#customevent
[DEP0025 warning]: deprecations.md#dep0025-requirenodesys
[ECMAScript module]: esm.md#modules-ecmascript-modules
[ExperimentalWarning: `vm.measureMemory` is an experimental feature]: vm.md#vmmeasurememoryoptions
[Fetch API]: https://developer.mozilla.org/en-US/docs/Web/API/Fetch_API
[File System Permissions]: permissions.md#file-system-permissions
[Module customization hooks]: module.md#customization-hooks
[Module customization hooks: enabling]: module.md#enabling
[Modules loaders]: packages.md#modules-loaders
[Navigator API]: globals.md#navigator
[Node.js issue tracker]: https://github.com/nodejs/node/issues
[OSSL_PROVIDER-legacy]: https://www.openssl.org/docs/man3.0/man7/OSSL_PROVIDER-legacy.html
[Permission Model]: permissions.md#permission-model
[REPL]: repl.md
[ScriptCoverage]: https://chromedevtools.github.io/devtools-protocol/tot/Profiler#type-ScriptCoverage
[ShadowRealm]: https://github.com/tc39/proposal-shadowrealm
[Source Map]: https://sourcemaps.info/spec.html
[Subresource Integrity]: https://developer.mozilla.org/en-US/docs/Web/Security/Subresource_Integrity
[V8 JavaScript code coverage]: https://v8project.blogspot.com/2017/12/javascript-code-coverage.html
[Web Crypto API]: webcrypto.md
[`"type"`]: packages.md#type
[`--allow-child-process`]: #--allow-child-process
[`--allow-fs-read`]: #--allow-fs-read
[`--allow-fs-write`]: #--allow-fs-write
[`--allow-worker`]: #--allow-worker
[`--build-snapshot`]: #--build-snapshot
[`--cpu-prof-dir`]: #--cpu-prof-dir
[`--diagnostic-dir`]: #--diagnostic-dirdirectory
[`--experimental-default-type=module`]: #--experimental-default-typetype
[`--experimental-sea-config`]: single-executable-applications.md#generating-single-executable-preparation-blobs
[`--experimental-wasm-modules`]: #--experimental-wasm-modules
[`--heap-prof-dir`]: #--heap-prof-dir
[`--import`]: #--importmodule
[`--openssl-config`]: #--openssl-configfile
[`--preserve-symlinks`]: #--preserve-symlinks
[`--print`]: #-p---print-script
[`--redirect-warnings`]: #--redirect-warningsfile
[`--require`]: #-r---require-module
[`Atomics.wait()`]: https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Atomics/wait
[`Buffer`]: buffer.md#class-buffer
[`CRYPTO_secure_malloc_init`]: https://www.openssl.org/docs/man3.0/man3/CRYPTO_secure_malloc_init.html
[`NODE_OPTIONS`]: #node_optionsoptions
[`NO_COLOR`]: https://no-color.org
[`SlowBuffer`]: buffer.md#class-slowbuffer
[`WebSocket`]: https://developer.mozilla.org/en-US/docs/Web/API/WebSocket
[`YoungGenerationSizeFromSemiSpaceSize`]: https://chromium.googlesource.com/v8/v8.git/+/refs/tags/10.3.129/src/heap/heap.cc#328
[`dns.lookup()`]: dns.md#dnslookuphostname-options-callback
[`dns.setDefaultResultOrder()`]: dns.md#dnssetdefaultresultorderorder
[`dnsPromises.lookup()`]: dns.md#dnspromiseslookuphostname-options
[`import` specifier]: esm.md#import-specifiers
[`process.setUncaughtExceptionCaptureCallback()`]: process.md#processsetuncaughtexceptioncapturecallbackfn
[`process.setuid()`]: process.md#processsetuidid
[`setuid(2)`]: https://man7.org/linux/man-pages/man2/setuid.2.html
[`tls.DEFAULT_MAX_VERSION`]: tls.md#tlsdefault_max_version
[`tls.DEFAULT_MIN_VERSION`]: tls.md#tlsdefault_min_version
[`unhandledRejection`]: process.md#event-unhandledrejection
[`v8.startupSnapshot` API]: v8.md#startup-snapshot-api
[`worker_threads.threadId`]: worker_threads.md#workerthreadid
[collecting code coverage from tests]: test.md#collecting-code-coverage
[conditional exports]: packages.md#conditional-exports
[context-aware]: addons.md#context-aware-addons
[debugger]: debugger.md
[debugging security implications]: https://nodejs.org/en/docs/guides/debugging-getting-started/#security-implications
[deprecation warnings]: deprecations.md#list-of-deprecated-apis
[emit_warning]: process.md#processemitwarningwarning-options
[environment_variables]: #environment-variables
[filtering tests by name]: test.md#filtering-tests-by-name
[jitless]: https://v8.dev/blog/jitless
[libuv threadpool documentation]: https://docs.libuv.org/en/latest/threadpool.html
[remote code execution]: https://www.owasp.org/index.php/Code_Injection
[running tests from the command line]: test.md#running-tests-from-the-command-line
[scavenge garbage collector]: https://v8.dev/blog/orinoco-parallel-scavenger
[security warning]: #warning-binding-inspector-to-a-public-ipport-combination-is-insecure
[semi-space]: https://www.memorymanagement.org/glossary/s.html#semi.space
[single executable application]: single-executable-applications.md
[test reporters]: test.md#test-reporters
[timezone IDs]: https://en.wikipedia.org/wiki/List_of_tz_database_time_zones
[tracking issue for user-land snapshots]: https://github.com/nodejs/node/issues/44014
[ways that `TZ` is handled in other environments]: https://www.gnu.org/software/libc/manual/html_node/TZ-Variable.html
      "data": {
        "version": 3,
        "sources": [
          "file:///absolute/path/to/original.js"
        ],
        "names": [
          "Foo",
          "console",
          "info"
        ],
        "mappings": "MAAMA,IACJC,YAAaC",
        "sourceRoot": "./"
      },
      "lineLengths": [
        13,
        62,
        38,
        27
      ]
    }
  }
}
```

### `OPENSSL_CONF=file`

<!-- YAML
added: v6.11.0
-->

Load an OpenSSL configuration file on startup. Among other uses, this can be
used to enable FIPS-compliant crypto if Node.js is built with
`./configure --openssl-fips`.

If the [`--openssl-config`][] command-line option is used, the environment
variable is ignored.

### `SSL_CERT_DIR=dir`

<!-- YAML
added: v7.7.0
-->

If `--use-openssl-ca` is enabled, this overrides and sets OpenSSL's directory
containing trusted certificates.

Be aware that unless the child environment is explicitly set, this environment
variable will be inherited by any child processes, and if they use OpenSSL, it
may cause them to trust the same CAs as node.

### `SSL_CERT_FILE=file`

<!-- YAML
added: v7.7.0
-->

If `--use-openssl-ca` is enabled, this overrides and sets OpenSSL's file
containing trusted certificates.

Be aware that unless the child environment is explicitly set, this environment
variable will be inherited by any child processes, and if they use OpenSSL, it
may cause them to trust the same CAs as node.

### `TZ`

<!-- YAML
added: v0.0.1
changes:
  - version:
     - v16.2.0
    pr-url: https://github.com/nodejs/node/pull/38642
    description:
      Changing the TZ variable using process.env.TZ = changes the timezone
      on Windows as well.
  - version:
     - v13.0.0
    pr-url: https://github.com/nodejs/node/pull/20026
    description:
      Changing the TZ variable using process.env.TZ = changes the timezone
      on POSIX systems.
-->

The `TZ` environment variable is used to specify the timezone configuration.

While Node.js does not support all of the various [ways that `TZ` is handled in
other environments][], it does support basic [timezone IDs][] (such as
`'Etc/UTC'`, `'Europe/Paris'`, or `'America/New_York'`).
It may support a few other abbreviations or aliases, but these are strongly
discouraged and not guaranteed.

```console
$ TZ=Europe/Dublin node -pe "new Date().toString()"
Wed May 12 2021 20:30:48 GMT+0100 (Irish Standard Time)
```

### `UV_THREADPOOL_SIZE=size`

Set the number of threads used in libuv's threadpool to `size` threads.

Asynchronous system APIs are used by Node.js whenever possible, but where they
do not exist, libuv's threadpool is used to create asynchronous node APIs based
on synchronous system APIs. Node.js APIs that use the threadpool are:

* all `fs` APIs, other than the file watcher APIs and those that are explicitly
  synchronous
* asynchronous crypto APIs such as `crypto.pbkdf2()`, `crypto.scrypt()`,
  `crypto.randomBytes()`, `crypto.randomFill()`, `crypto.generateKeyPair()`
* `dns.lookup()`
* all `zlib` APIs, other than those that are explicitly synchronous

Because libuv's threadpool has a fixed size, it means that if for whatever
reason any of these APIs takes a long time, other (seemingly unrelated) APIs
that run in libuv's threadpool will experience degraded performance. In order to
mitigate this issue, one potential solution is to increase the size of libuv's
threadpool by setting the `'UV_THREADPOOL_SIZE'` environment variable to a value
greater than `4` (its current default value). For more information, see the
[libuv threadpool documentation][].

### `UV_USE_IO_URING=value`

Enable or disable libuv's use of `io_uring` on supported platforms.

On supported platforms, `io_uring` can significantly improve the performance of
various asynchronous I/O operations.

`io_uring` is disabled by default due to security concerns. When `io_uring`
is enabled, applications must not change the user identity of the process at
runtime, neither through JavaScript functions such as [`process.setuid()`][] nor
through native addons that can invoke system functions such as [`setuid(2)`][].

This environment variable is implemented by a dependency of Node.js and may be
removed in future versions of Node.js. No stability guarantees are provided for
the behavior of this environment variable.

## Useful V8 options

V8 has its own set of CLI options. Any V8 CLI option that is provided to `node`
will be passed on to V8 to handle. V8's options have _no stability guarantee_.
The V8 team themselves don't consider them to be part of their formal API,
and reserve the right to change them at any time. Likewise, they are not
covered by the Node.js stability guarantees. Many of the V8
options are of interest only to V8 developers. Despite this, there is a small
set of V8 options that are widely applicable to Node.js, and they are
documented here:

### `--max-old-space-size=SIZE` (in megabytes)

Sets the max memory size of V8's old memory section. As memory
consumption approaches the limit, V8 will spend more time on
garbage collection in an effort to free unused memory.

On a machine with 2 GiB of memory, consider setting this to
1536 (1.5 GiB) to leave some memory for other uses and avoid swapping.

```bash
node --max-old-space-size=1536 index.js
```

### `--max-semi-space-size=SIZE` (in megabytes)

Sets the maximum [semi-space][] size for V8's [scavenge garbage collector][] in
MiB (megabytes).
Increasing the max size of a semi-space may improve throughput for Node.js at
the cost of more memory consumption.

Since the young generation size of the V8 heap is three times (see
[`YoungGenerationSizeFromSemiSpaceSize`][] in V8) the size of the semi-space,
an increase of 1 MiB to semi-space applies to each of the three individual
semi-spaces and causes the heap size to increase by 3 MiB. The throughput
improvement depends on your workload (see [#42511][]).

The default value is 16 MiB for 64-bit systems and 8 MiB for 32-bit systems. To
get the best configuration for your application, you should try different
max-semi-space-size values when running benchmarks for your application.

For example, benchmark on a 64-bit systems:

```bash
for MiB in 16 32 64 128; do
    node --max-semi-space-size=$MiB index.js
done
```

[#42511]: https://github.com/nodejs/node/issues/42511
[Chrome DevTools Protocol]: https://chromedevtools.github.io/devtools-protocol/
[CommonJS]: modules.md
[CommonJS module]: modules.md
[CustomEvent Web API]: https://dom.spec.whatwg.org/#customevent
[DEP0025 warning]: deprecations.md#dep0025-requirenodesys
[ECMAScript module]: esm.md#modules-ecmascript-modules
[ExperimentalWarning: `vm.measureMemory` is an experimental feature]: vm.md#vmmeasurememoryoptions
[Fetch API]: https://developer.mozilla.org/en-US/docs/Web/API/Fetch_API
[File System Permissions]: permissions.md#file-system-permissions
[Module customization hooks]: module.md#customization-hooks
[Module customization hooks: enabling]: module.md#enabling
[Modules loaders]: packages.md#modules-loaders
[Navigator API]: globals.md#navigator
[Node.js issue tracker]: https://github.com/nodejs/node/issues
[OSSL_PROVIDER-legacy]: https://www.openssl.org/docs/man3.0/man7/OSSL_PROVIDER-legacy.html
[Permission Model]: permissions.md#permission-model
[REPL]: repl.md
[ScriptCoverage]: https://chromedevtools.github.io/devtools-protocol/tot/Profiler#type-ScriptCoverage
[ShadowRealm]: https://github.com/tc39/proposal-shadowrealm
[Source Map]: https://sourcemaps.info/spec.html
[Subresource Integrity]: https://developer.mozilla.org/en-US/docs/Web/Security/Subresource_Integrity
[V8 JavaScript code coverage]: https://v8project.blogspot.com/2017/12/javascript-code-coverage.html
[Web Crypto API]: webcrypto.md
[`"type"`]: packages.md#type
[`--allow-child-process`]: #--allow-child-process
[`--allow-fs-read`]: #--allow-fs-read
[`--allow-fs-write`]: #--allow-fs-write
[`--allow-worker`]: #--allow-worker
[`--build-snapshot`]: #--build-snapshot
[`--cpu-prof-dir`]: #--cpu-prof-dir
[`--diagnostic-dir`]: #--diagnostic-dirdirectory
[`--experimental-default-type=module`]: #--experimental-default-typetype
[`--experimental-sea-config`]: single-executable-applications.md#generating-single-executable-preparation-blobs
[`--experimental-wasm-modules`]: #--experimental-wasm-modules
[`--heap-prof-dir`]: #--heap-prof-dir
[`--import`]: #--importmodule
[`--openssl-config`]: #--openssl-configfile
[`--preserve-symlinks`]: #--preserve-symlinks
[`--print`]: #-p---print-script
[`--redirect-warnings`]: #--redirect-warningsfile
[`--require`]: #-r---require-module
[`Atomics.wait()`]: https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Atomics/wait
[`Buffer`]: buffer.md#class-buffer
[`CRYPTO_secure_malloc_init`]: https://www.openssl.org/docs/man3.0/man3/CRYPTO_secure_malloc_init.html
[`NODE_OPTIONS`]: #node_optionsoptions
[`NO_COLOR`]: https://no-color.org
[`SlowBuffer`]: buffer.md#class-slowbuffer
[`WebSocket`]: https://developer.mozilla.org/en-US/docs/Web/API/WebSocket
[`YoungGenerationSizeFromSemiSpaceSize`]: https://chromium.googlesource.com/v8/v8.git/+/refs/tags/10.3.129/src/heap/heap.cc#328
[`dns.lookup()`]: dns.md#dnslookuphostname-options-callback
[`dns.setDefaultResultOrder()`]: dns.md#dnssetdefaultresultorderorder
[`dnsPromises.lookup()`]: dns.md#dnspromiseslookuphostname-options
[`import` specifier]: esm.md#import-specifiers
[`process.setUncaughtExceptionCaptureCallback()`]: process.md#processsetuncaughtexceptioncapturecallbackfn
[`process.setuid()`]: process.md#processsetuidid
[`setuid(2)`]: https://man7.org/linux/man-pages/man2/setuid.2.html
[`tls.DEFAULT_MAX_VERSION`]: tls.md#tlsdefault_max_version
[`tls.DEFAULT_MIN_VERSION`]: tls.md#tlsdefault_min_version
[`unhandledRejection`]: process.md#event-unhandledrejection
[`v8.startupSnapshot` API]: v8.md#startup-snapshot-api
[`worker_threads.threadId`]: worker_threads.md#workerthreadid
[collecting code coverage from tests]: test.md#collecting-code-coverage
[conditional exports]: packages.md#conditional-exports
[context-aware]: addons.md#context-aware-addons
[debugger]: debugger.md
[debugging security implications]: https://nodejs.org/en/docs/guides/debugging-getting-started/#security-implications
[deprecation warnings]: deprecations.md#list-of-deprecated-apis
[emit_warning]: process.md#processemitwarningwarning-options
[environment_variables]: #environment-variables
[filtering tests by name]: test.md#filtering-tests-by-name
[jitless]: https://v8.dev/blog/jitless
[libuv threadpool documentation]: https://docs.libuv.org/en/latest/threadpool.html
[remote code execution]: https://www.owasp.org/index.php/Code_Injection
[running tests from the command line]: test.md#running-tests-from-the-command-line
[scavenge garbage collector]: https://v8.dev/blog/orinoco-parallel-scavenger
[security warning]: #warning-binding-inspector-to-a-public-ipport-combination-is-insecure
[semi-space]: https://www.memorymanagement.org/glossary/s.html#semi.space
[single executable application]: single-executable-applications.md
[test reporters]: test.md#test-reporters
[timezone IDs]: https://en.wikipedia.org/wiki/List_of_tz_database_time_zones
[tracking issue for user-land snapshots]: https://github.com/nodejs/node/issues/44014
[ways that `TZ` is handled in other environments]: https://www.gnu.org/software/libc/manual/html_node/TZ-Variable.html