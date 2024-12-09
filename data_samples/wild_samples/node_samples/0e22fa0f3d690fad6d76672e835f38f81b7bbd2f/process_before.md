  } catch (err) {
    console.log(`Failed to set uid: ${err}`);
  }
}
```

This function is only available on POSIX platforms (i.e. not Windows or
Android).
This feature is not available in [`Worker`][] threads.

## `process.setSourceMapsEnabled(val)`
<!-- YAML
added: REPLACEME
-->

> Stability: 1 - Experimental

* `val` {boolean}

This function enables or disables the [Source Map v3][Source Map] support for
stack traces.

It provides same features as launching Node.js process with commandline options
`--enable-source-maps`.

Only source maps in JavaScript files that are loaded after source maps has been
enabled will be parsed and loaded.

## `process.setUncaughtExceptionCaptureCallback(fn)`
<!-- YAML
added: v9.3.0
-->

* `fn` {Function|null}

The `process.setUncaughtExceptionCaptureCallback()` function sets a function
that will be invoked when an uncaught exception occurs, which will receive the
exception value itself as its first argument.

If such a function is set, the [`'uncaughtException'`][] event will
not be emitted. If `--abort-on-uncaught-exception` was passed from the
command line or set through [`v8.setFlagsFromString()`][], the process will
not abort. Actions configured to take place on exceptions such as report
generations will be affected too

To unset the capture function,
`process.setUncaughtExceptionCaptureCallback(null)` may be used. Calling this
method with a non-`null` argument while another capture function is set will
throw an error.

Using this function is mutually exclusive with using the deprecated
[`domain`][] built-in module.

## `process.stderr`

* {Stream}

The `process.stderr` property returns a stream connected to
`stderr` (fd `2`). It is a [`net.Socket`][] (which is a [Duplex][]
stream) unless fd `2` refers to a file, in which case it is
a [Writable][] stream.

`process.stderr` differs from other Node.js streams in important ways. See
[note on process I/O][] for more information.

### `process.stderr.fd`

* {number}

This property refers to the value of underlying file descriptor of
`process.stderr`. The value is fixed at `2`. In [`Worker`][] threads,
this field does not exist.

## `process.stdin`

* {Stream}

The `process.stdin` property returns a stream connected to
`stdin` (fd `0`). It is a [`net.Socket`][] (which is a [Duplex][]
stream) unless fd `0` refers to a file, in which case it is
a [Readable][] stream.

For details of how to read from `stdin` see [`readable.read()`][].

As a [Duplex][] stream, `process.stdin` can also be used in "old" mode that
is compatible with scripts written for Node.js prior to v0.10.
For more information see [Stream compatibility][].

In "old" streams mode the `stdin` stream is paused by default, so one
must call `process.stdin.resume()` to read from it. Note also that calling
`process.stdin.resume()` itself would switch stream to "old" mode.

### `process.stdin.fd`

* {number}

This property refers to the value of underlying file descriptor of
`process.stdin`. The value is fixed at `0`. In [`Worker`][] threads,
this field does not exist.

## `process.stdout`

* {Stream}

The `process.stdout` property returns a stream connected to
`stdout` (fd `1`). It is a [`net.Socket`][] (which is a [Duplex][]
stream) unless fd `1` refers to a file, in which case it is
a [Writable][] stream.

For example, to copy `process.stdin` to `process.stdout`:

```mjs
import { stdin, stdout } from 'process';

stdin.pipe(stdout);
```

```cjs
const { stdin, stdout } = require('process');

stdin.pipe(stdout);
```

`process.stdout` differs from other Node.js streams in important ways. See
[note on process I/O][] for more information.

### `process.stdout.fd`

* {number}

This property refers to the value of underlying file descriptor of
`process.stdout`. The value is fixed at `1`. In [`Worker`][] threads,
this field does not exist.

### A note on process I/O

`process.stdout` and `process.stderr` differ from other Node.js streams in
important ways:

1. They are used internally by [`console.log()`][] and [`console.error()`][],
   respectively.
2. Writes may be synchronous depending on what the stream is connected to
   and whether the system is Windows or POSIX:
   * Files: *synchronous* on Windows and POSIX
   * TTYs (Terminals): *asynchronous* on Windows, *synchronous* on POSIX
   * Pipes (and sockets): *synchronous* on Windows, *asynchronous* on POSIX

These behaviors are partly for historical reasons, as changing them would
create backward incompatibility, but they are also expected by some users.

Synchronous writes avoid problems such as output written with `console.log()` or
`console.error()` being unexpectedly interleaved, or not written at all if
`process.exit()` is called before an asynchronous write completes. See
[`process.exit()`][] for more information.

***Warning***: Synchronous writes block the event loop until the write has
completed. This can be near instantaneous in the case of output to a file, but
under high system load, pipes that are not being read at the receiving end, or
with slow terminals or file systems, its possible for the event loop to be
blocked often enough and long enough to have severe negative performance
impacts. This may not be a problem when writing to an interactive terminal
session, but consider this particularly careful when doing production logging to
the process output streams.

To check if a stream is connected to a [TTY][] context, check the `isTTY`
property.

For instance:

```console
$ node -p "Boolean(process.stdin.isTTY)"
true
$ echo "foo" | node -p "Boolean(process.stdin.isTTY)"
false
$ node -p "Boolean(process.stdout.isTTY)"
true
$ node -p "Boolean(process.stdout.isTTY)" | cat
false
```

See the [TTY][] documentation for more information.

## `process.throwDeprecation`
<!-- YAML
added: v0.9.12
-->

* {boolean}

The initial value of `process.throwDeprecation` indicates whether the
`--throw-deprecation` flag is set on the current Node.js process.
`process.throwDeprecation` is mutable, so whether or not deprecation
warnings result in errors may be altered at runtime. See the
documentation for the [`'warning'` event][process_warning] and the
[`emitWarning()` method][process_emit_warning] for more information.

```console
$ node --throw-deprecation -p "process.throwDeprecation"
true
$ node -p "process.throwDeprecation"
undefined
$ node
> process.emitWarning('test', 'DeprecationWarning');
undefined
> (node:26598) DeprecationWarning: test
> process.throwDeprecation = true;
true
> process.emitWarning('test', 'DeprecationWarning');
Thrown:
[DeprecationWarning: test] { name: 'DeprecationWarning' }
```

## `process.title`
<!-- YAML
added: v0.1.104
-->

* {string}

The `process.title` property returns the current process title (i.e. returns
the current value of `ps`). Assigning a new value to `process.title` modifies
the current value of `ps`.

When a new value is assigned, different platforms will impose different maximum
length restrictions on the title. Usually such restrictions are quite limited.
For instance, on Linux and macOS, `process.title` is limited to the size of the
binary name plus the length of the command-line arguments because setting the
`process.title` overwrites the `argv` memory of the process. Node.js v0.8
allowed for longer process title strings by also overwriting the `environ`
memory but that was potentially insecure and confusing in some (rather obscure)
cases.

Assigning a value to `process.title` might not result in an accurate label
within process manager applications such as macOS Activity Monitor or Windows
Services Manager.

## `process.traceDeprecation`
<!-- YAML
added: v0.8.0
-->

* {boolean}

The `process.traceDeprecation` property indicates whether the
`--trace-deprecation` flag is set on the current Node.js process. See the
documentation for the [`'warning'` event][process_warning] and the
[`emitWarning()` method][process_emit_warning] for more information about this
flag's behavior.

## `process.umask()`
<!-- YAML
added: v0.1.19
changes:
  - version:
    - v14.0.0
    - v12.19.0
    pr-url: https://github.com/nodejs/node/pull/32499
    description: Calling `process.umask()` with no arguments is deprecated.

-->

> Stability: 0 - Deprecated. Calling `process.umask()` with no argument causes
> the process-wide umask to be written twice. This introduces a race condition
> between threads, and is a potential security vulnerability. There is no safe,
> cross-platform alternative API.

`process.umask()` returns the Node.js process's file mode creation mask. Child
processes inherit the mask from the parent process.

## `process.umask(mask)`
<!-- YAML
added: v0.1.19
-->

* `mask` {string|integer}

`process.umask(mask)` sets the Node.js process's file mode creation mask. Child
processes inherit the mask from the parent process. Returns the previous mask.

```mjs
import { umask } from 'process';

const newmask = 0o022;
const oldmask = umask(newmask);
console.log(
  `Changed umask from ${oldmask.toString(8)} to ${newmask.toString(8)}`
);
```

```cjs
const { umask } = require('process');

const newmask = 0o022;
const oldmask = umask(newmask);
console.log(
  `Changed umask from ${oldmask.toString(8)} to ${newmask.toString(8)}`
);
```

In [`Worker`][] threads, `process.umask(mask)` will throw an exception.

## `process.uptime()`
<!-- YAML
added: v0.5.0
-->

* Returns: {number}

The `process.uptime()` method returns the number of seconds the current Node.js
process has been running.

The return value includes fractions of a second. Use `Math.floor()` to get whole
seconds.

## `process.version`
<!-- YAML
added: v0.1.3
-->

* {string}

The `process.version` property contains the Node.js version string.

```mjs
import { version } from 'process';

console.log(`Version: ${version}`);
// Version: v14.8.0
```

```cjs
const { version } = require('process');

console.log(`Version: ${version}`);
// Version: v14.8.0
```

To get the version string without the prepended _v_, use
`process.versions.node`.

## `process.versions`
<!-- YAML
added: v0.2.0
changes:
  - version: v9.0.0
    pr-url: https://github.com/nodejs/node/pull/15785
    description: The `v8` property now includes a Node.js specific suffix.
  - version: v4.2.0
    pr-url: https://github.com/nodejs/node/pull/3102
    description: The `icu` property is now supported.
-->

* {Object}

The `process.versions` property returns an object listing the version strings of
Node.js and its dependencies. `process.versions.modules` indicates the current
ABI version, which is increased whenever a C++ API changes. Node.js will refuse
to load modules that were compiled against a different module ABI version.

```mjs
import { versions } from 'process';

console.log(versions);
```

```cjs
const { versions } = require('process');

console.log(versions);
```

Will generate an object similar to:

```console
{ node: '11.13.0',
  v8: '7.0.276.38-node.18',
  uv: '1.27.0',
  zlib: '1.2.11',
  brotli: '1.0.7',
  ares: '1.15.0',
  modules: '67',
  nghttp2: '1.34.0',
  napi: '4',
  llhttp: '1.1.1',
  openssl: '1.1.1b',
  cldr: '34.0',
  icu: '63.1',
  tz: '2018e',
  unicode: '11.0' }
```

## Exit codes

Node.js will normally exit with a `0` status code when no more async
operations are pending. The following status codes are used in other
cases:

* `1` **Uncaught Fatal Exception**: There was an uncaught exception,
  and it was not handled by a domain or an [`'uncaughtException'`][] event
  handler.
* `2`: Unused (reserved by Bash for builtin misuse)
* `3` **Internal JavaScript Parse Error**: The JavaScript source code
  internal in the Node.js bootstrapping process caused a parse error. This
  is extremely rare, and generally can only happen during development
  of Node.js itself.
* `4` **Internal JavaScript Evaluation Failure**: The JavaScript
  source code internal in the Node.js bootstrapping process failed to
  return a function value when evaluated. This is extremely rare, and
  generally can only happen during development of Node.js itself.
* `5` **Fatal Error**: There was a fatal unrecoverable error in V8.
  Typically a message will be printed to stderr with the prefix `FATAL
  ERROR`.
* `6` **Non-function Internal Exception Handler**: There was an
  uncaught exception, but the internal fatal exception handler
  function was somehow set to a non-function, and could not be called.
* `7` **Internal Exception Handler Run-Time Failure**: There was an
  uncaught exception, and the internal fatal exception handler
  function itself threw an error while attempting to handle it. This
  can happen, for example, if an [`'uncaughtException'`][] or
  `domain.on('error')` handler throws an error.
* `8`: Unused. In previous versions of Node.js, exit code 8 sometimes
  indicated an uncaught exception.
* `9` **Invalid Argument**: Either an unknown option was specified,
  or an option requiring a value was provided without a value.
* `10` **Internal JavaScript Run-Time Failure**: The JavaScript
  source code internal in the Node.js bootstrapping process threw an error
  when the bootstrapping function was called. This is extremely rare,
  and generally can only happen during development of Node.js itself.
* `12` **Invalid Debug Argument**: The `--inspect` and/or `--inspect-brk`
  options were set, but the port number chosen was invalid or unavailable.
* `13` **Unfinished Top-Level Await**: `await` was used outside of a function
  in the top-level code, but the passed `Promise` never resolved.
* `>128` **Signal Exits**: If Node.js receives a fatal signal such as
  `SIGKILL` or `SIGHUP`, then its exit code will be `128` plus the
  value of the signal code. This is a standard POSIX practice, since
  exit codes are defined to be 7-bit integers, and signal exits set
  the high-order bit, and then contain the value of the signal code.
  For example, signal `SIGABRT` has value `6`, so the expected exit
  code will be `128` + `6`, or `134`.

[Advanced serialization for `child_process`]: child_process.md#child_process_advanced_serialization
[Android building]: https://github.com/nodejs/node/blob/HEAD/BUILDING.md#androidandroid-based-devices-eg-firefox-os
[Child Process]: child_process.md
[Cluster]: cluster.md
[Duplex]: stream.md#stream_duplex_and_transform_streams
[Event Loop]: https://nodejs.org/en/docs/guides/event-loop-timers-and-nexttick/#process-nexttick
[LTS]: https://github.com/nodejs/Release
[Readable]: stream.md#stream_readable_streams
[Signal Events]: #process_signal_events
[Source Map]: https://sourcemaps.info/spec.html
[Stream compatibility]: stream.md#stream_compatibility_with_older_node_js_versions
[TTY]: tty.md#tty_tty
[Writable]: stream.md#stream_writable_streams
[`'exit'`]: #process_event_exit
[`'message'`]: child_process.md#child_process_event_message
[`'uncaughtException'`]: #process_event_uncaughtexception
[`--unhandled-rejections`]: cli.md#cli_unhandled_rejections_mode
[`Buffer`]: buffer.md
[`ChildProcess.disconnect()`]: child_process.md#child_process_subprocess_disconnect
[`ChildProcess.send()`]: child_process.md#child_process_subprocess_send_message_sendhandle_options_callback
[`ChildProcess`]: child_process.md#child_process_class_childprocess
[`Error`]: errors.md#errors_class_error
[`EventEmitter`]: events.md#events_class_eventemitter
[`NODE_OPTIONS`]: cli.md#cli_node_options_options
[`Promise.race()`]: https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Promise/race
[`Worker`]: worker_threads.md#worker_threads_class_worker
[`Worker` constructor]: worker_threads.md#worker_threads_new_worker_filename_options
[`console.error()`]: console.md#console_console_error_data_args
[`console.log()`]: console.md#console_console_log_data_args
[`domain`]: domain.md
[`net.Server`]: net.md#net_class_net_server
[`net.Socket`]: net.md#net_class_net_socket
[`os.constants.dlopen`]: os.md#os_dlopen_constants
[`process.argv`]: #process_process_argv
[`process.config`]: #process_process_config
[`process.execPath`]: #process_process_execpath
[`process.exit()`]: #process_process_exit_code
[`process.exitCode`]: #process_process_exitcode
[`process.hrtime()`]: #process_process_hrtime_time
[`process.hrtime.bigint()`]: #process_process_hrtime_bigint
[`process.kill()`]: #process_process_kill_pid_signal
[`process.setUncaughtExceptionCaptureCallback()`]: #process_process_setuncaughtexceptioncapturecallback_fn
[`promise.catch()`]: https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Promise/catch
[`queueMicrotask()`]: globals.md#globals_queuemicrotask_callback
[`readable.read()`]: stream.md#stream_readable_read_size
[`require()`]: globals.md#globals_require
[`require.main`]: modules.md#modules_accessing_the_main_module
[`subprocess.kill()`]: child_process.md#child_process_subprocess_kill_signal
[`v8.setFlagsFromString()`]: v8.md#v8_v8_setflagsfromstring_flags
[debugger]: debugger.md
[deprecation code]: deprecations.md
[note on process I/O]: #process_a_note_on_process_i_o
[process.cpuUsage]: #process_process_cpuusage_previousvalue
[process_emit_warning]: #process_process_emitwarning_warning_type_code_ctor
[process_warning]: #process_event_warning
[report documentation]: report.md
[terminal raw mode]: tty.md#tty_readstream_setrawmode_mode
[uv_rusage_t]: https://docs.libuv.org/en/v1.x/misc.html#c.uv_rusage_t
[wikipedia_major_fault]: https://en.wikipedia.org/wiki/Page_fault#Major
[wikipedia_minor_fault]: https://en.wikipedia.org/wiki/Page_fault#Minor