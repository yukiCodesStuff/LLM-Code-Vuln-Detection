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
[`dnsPromises.lookup()`]: dns.md#dnspromiseslookuphostname-options
[`import` specifier]: esm.md#import-specifiers
[`process.setUncaughtExceptionCaptureCallback()`]: process.md#processsetuncaughtexceptioncapturecallbackfn
[`process.setuid()`]: process.md#processsetuidid
[`setuid(2)`]: https://man7.org/linux/man-pages/man2/setuid.2.html
[`tls.DEFAULT_MAX_VERSION`]: tls.md#tlsdefault_max_version
[`tls.DEFAULT_MIN_VERSION`]: tls.md#tlsdefault_min_version
[`unhandledRejection`]: process.md#event-unhandledrejection
[`v8.startupSnapshot` API]: v8.md#startup-snapshot-api