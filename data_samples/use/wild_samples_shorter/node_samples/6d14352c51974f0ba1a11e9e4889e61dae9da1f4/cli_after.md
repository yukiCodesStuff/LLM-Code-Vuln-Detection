
`io_uring` is disabled by default due to security concerns. When `io_uring`
is enabled, applications must not change the user identity of the process at
runtime. In this case, JavaScript functions such as [`process.setuid()`][] are
unavailable, and native addons must not invoke system functions such as
[`setuid(2)`][].

This environment variable is implemented by a dependency of Node.js and may be
removed in future versions of Node.js. No stability guarantees are provided for
the behavior of this environment variable.