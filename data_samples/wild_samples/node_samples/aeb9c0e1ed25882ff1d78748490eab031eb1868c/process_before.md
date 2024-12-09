if (someConditionNotMet()) {
  printUsageToStdout();
  process.exitCode = 1;
}
```

If it is necessary to terminate the Node.js process due to an error condition,
throwing an _uncaught_ error and allowing the process to terminate accordingly
is safer than calling `process.exit()`.

In [`Worker`][] threads, this function stops the current thread rather
than the current process.

## `process.exitCode`

<!-- YAML
added: v0.11.8
-->

* {integer}

A number which will be the process exit code, when the process either
exits gracefully, or is exited via [`process.exit()`][] without specifying
a code.

Specifying a code to [`process.exit(code)`][`process.exit()`] will override any
previous setting of `process.exitCode`.

## `process.getActiveResourcesInfo()`

<!-- YAML
added: REPLACEME
-->

> Stability: 1 - Experimental

* Returns: {string\[]}

The `process.getActiveResourcesInfo()` method returns an array of strings
containing the types of the active resources that are currently keeping the
event loop alive.

```mjs
import { getActiveResourcesInfo } from 'process';
import { setTimeout } from 'timers';

console.log('Before:', getActiveResourcesInfo());
setTimeout(() => {}, 1000);
console.log('After:', getActiveResourcesInfo());
// Prints:
//   Before: [ 'CloseReq', 'TTYWrap', 'TTYWrap', 'TTYWrap' ]
//   After: [ 'CloseReq', 'TTYWrap', 'TTYWrap', 'TTYWrap', 'Timeout' ]
```

```cjs
const { getActiveResourcesInfo } = require('process');
const { setTimeout } = require('timers');

console.log('Before:', getActiveResourcesInfo());
setTimeout(() => {}, 1000);
console.log('After:', getActiveResourcesInfo());
// Prints:
//   Before: [ 'TTYWrap', 'TTYWrap', 'TTYWrap' ]
//   After: [ 'TTYWrap', 'TTYWrap', 'TTYWrap', 'Timeout' ]
```

## `process.getegid()`

<!-- YAML
added: v2.0.0
-->

The `process.getegid()` method returns the numerical effective group identity
of the Node.js process. (See getegid(2).)

```mjs
import process from 'process';

if (process.getegid) {
  console.log(`Current gid: ${process.getegid()}`);
}
```

```cjs
const process = require('process');

if (process.getegid) {
  console.log(`Current gid: ${process.getegid()}`);
}
```

This function is only available on POSIX platforms (i.e. not Windows or
Android).

## `process.geteuid()`

<!-- YAML
added: v2.0.0
-->

* Returns: {Object}

The `process.geteuid()` method returns the numerical effective user identity of
the process. (See geteuid(2).)

```mjs
import process from 'process';

if (process.geteuid) {
  console.log(`Current uid: ${process.geteuid()}`);
}
```

```cjs
const process = require('process');

if (process.geteuid) {
  console.log(`Current uid: ${process.geteuid()}`);
}
```

This function is only available on POSIX platforms (i.e. not Windows or
Android).

## `process.getgid()`

<!-- YAML
added: v0.1.31
-->

* Returns: {Object}

The `process.getgid()` method returns the numerical group identity of the
process. (See getgid(2).)

```mjs
import process from 'process';

if (process.getgid) {
  console.log(`Current gid: ${process.getgid()}`);
}
```

```cjs
const process = require('process');

if (process.getgid) {
  console.log(`Current gid: ${process.getgid()}`);
}
```

This function is only available on POSIX platforms (i.e. not Windows or
Android).

## `process.getgroups()`

<!-- YAML
added: v0.9.4
-->

* Returns: {integer\[]}

The `process.getgroups()` method returns an array with the supplementary group
IDs. POSIX leaves it unspecified if the effective group ID is included but
Node.js ensures it always is.

```mjs
import process from 'process';

if (process.getgroups) {
  console.log(process.getgroups()); // [ 16, 21, 297 ]
}
```

```cjs
const process = require('process');

if (process.getgroups) {
  console.log(process.getgroups()); // [ 16, 21, 297 ]
}
```

This function is only available on POSIX platforms (i.e. not Windows or
Android).

## `process.getuid()`

<!-- YAML
added: v0.1.28
-->

* Returns: {integer}

The `process.getuid()` method returns the numeric user identity of the process.
(See getuid(2).)

```mjs
import process from 'process';

if (process.getuid) {
  console.log(`Current uid: ${process.getuid()}`);
}
```

```cjs
const process = require('process');

if (process.getuid) {
  console.log(`Current uid: ${process.getuid()}`);
}
```

This function is only available on POSIX platforms (i.e. not Windows or
Android).

## `process.hasUncaughtExceptionCaptureCallback()`

<!-- YAML
added: v9.3.0
-->

* Returns: {boolean}

Indicates whether a callback has been set using
[`process.setUncaughtExceptionCaptureCallback()`][].

## `process.hrtime([time])`

<!-- YAML
added: v0.7.6
-->

> Stability: 3 - Legacy. Use [`process.hrtime.bigint()`][] instead.

* `time` {integer\[]} The result of a previous call to `process.hrtime()`
* Returns: {integer\[]}

This is the legacy version of [`process.hrtime.bigint()`][]
before `bigint` was introduced in JavaScript.

The `process.hrtime()` method returns the current high-resolution real time
in a `[seconds, nanoseconds]` tuple `Array`, where `nanoseconds` is the
remaining part of the real time that can't be represented in second precision.

`time` is an optional parameter that must be the result of a previous
`process.hrtime()` call to diff with the current time. If the parameter
passed in is not a tuple `Array`, a `TypeError` will be thrown. Passing in a
user-defined array instead of the result of a previous call to
`process.hrtime()` will lead to undefined behavior.

These times are relative to an arbitrary time in the
past, and not related to the time of day and therefore not subject to clock
drift. The primary use is for measuring performance between intervals:

```mjs
import { hrtime } from 'process';

const NS_PER_SEC = 1e9;
const time = hrtime();
// [ 1800216, 25 ]

setTimeout(() => {
  const diff = hrtime(time);
  // [ 1, 552 ]

  console.log(`Benchmark took ${diff[0] * NS_PER_SEC + diff[1]} nanoseconds`);
  // Benchmark took 1000000552 nanoseconds
}, 1000);
```

```cjs
const { hrtime } = require('process');

const NS_PER_SEC = 1e9;
const time = hrtime();
// [ 1800216, 25 ]

setTimeout(() => {
  const diff = hrtime(time);
  // [ 1, 552 ]

  console.log(`Benchmark took ${diff[0] * NS_PER_SEC + diff[1]} nanoseconds`);
  // Benchmark took 1000000552 nanoseconds
}, 1000);
```

## `process.hrtime.bigint()`

<!-- YAML
added: v10.7.0
-->

* Returns: {bigint}

The `bigint` version of the [`process.hrtime()`][] method returning the
current high-resolution real time in nanoseconds as a `bigint`.

Unlike [`process.hrtime()`][], it does not support an additional `time`
argument since the difference can just be computed directly
by subtraction of the two `bigint`s.

```mjs
import { hrtime } from 'process';

const start = hrtime.bigint();
// 191051479007711n

setTimeout(() => {
  const end = hrtime.bigint();
  // 191052633396993n

  console.log(`Benchmark took ${end - start} nanoseconds`);
  // Benchmark took 1154389282 nanoseconds
}, 1000);
```

```cjs
const { hrtime } = require('process');

const start = hrtime.bigint();
// 191051479007711n

setTimeout(() => {
  const end = hrtime.bigint();
  // 191052633396993n

  console.log(`Benchmark took ${end - start} nanoseconds`);
  // Benchmark took 1154389282 nanoseconds
}, 1000);
```

## `process.initgroups(user, extraGroup)`

<!-- YAML
added: v0.9.4
-->

* `user` {string|number} The user name or numeric identifier.
* `extraGroup` {string|number} A group name or numeric identifier.

The `process.initgroups()` method reads the `/etc/group` file and initializes
the group access list, using all groups of which the user is a member. This is
a privileged operation that requires that the Node.js process either have `root`
access or the `CAP_SETGID` capability.

Use care when dropping privileges:

```mjs
import { getgroups, initgroups, setgid } from 'process';

console.log(getgroups());         // [ 0 ]
initgroups('nodeuser', 1000);     // switch user
console.log(getgroups());         // [ 27, 30, 46, 1000, 0 ]
setgid(1000);                     // drop root gid
console.log(getgroups());         // [ 27, 30, 46, 1000 ]
```

```cjs
const { getgroups, initgroups, setgid } = require('process');

console.log(getgroups());         // [ 0 ]
initgroups('nodeuser', 1000);     // switch user
console.log(getgroups());         // [ 27, 30, 46, 1000, 0 ]
setgid(1000);                     // drop root gid
console.log(getgroups());         // [ 27, 30, 46, 1000 ]
```

This function is only available on POSIX platforms (i.e. not Windows or
Android).
This feature is not available in [`Worker`][] threads.

## `process.kill(pid[, signal])`

<!-- YAML
added: v0.0.6
-->

* `pid` {number} A process ID
* `signal` {string|number} The signal to send, either as a string or number.
  **Default:** `'SIGTERM'`.

The `process.kill()` method sends the `signal` to the process identified by
`pid`.

Signal names are strings such as `'SIGINT'` or `'SIGHUP'`. See [Signal Events][]
and kill(2) for more information.

This method will throw an error if the target `pid` does not exist. As a special
case, a signal of `0` can be used to test for the existence of a process.
Windows platforms will throw an error if the `pid` is used to kill a process
group.

Even though the name of this function is `process.kill()`, it is really just a
signal sender, like the `kill` system call. The signal sent may do something
other than kill the target process.

```mjs
import process, { kill } from 'process';

process.on('SIGHUP', () => {
  console.log('Got SIGHUP signal.');
});

setTimeout(() => {
  console.log('Exiting.');
  process.exit(0);
}, 100);

kill(process.pid, 'SIGHUP');
```

```cjs
const process = require('process');

process.on('SIGHUP', () => {
  console.log('Got SIGHUP signal.');
});

setTimeout(() => {
  console.log('Exiting.');
  process.exit(0);
}, 100);

process.kill(process.pid, 'SIGHUP');
```

When `SIGUSR1` is received by a Node.js process, Node.js will start the
debugger. See [Signal Events][].

## `process.mainModule`

<!-- YAML
added: v0.1.17
deprecated: v14.0.0
-->

> Stability: 0 - Deprecated: Use [`require.main`][] instead.

* {Object}

The `process.mainModule` property provides an alternative way of retrieving
[`require.main`][]. The difference is that if the main module changes at
runtime, [`require.main`][] may still refer to the original main module in
modules that were required before the change occurred. Generally, it's
safe to assume that the two refer to the same module.

As with [`require.main`][], `process.mainModule` will be `undefined` if there
is no entry script.

## `process.memoryUsage()`

<!-- YAML
added: v0.1.16
changes:
  - version:
     - v13.9.0
     - v12.17.0
    pr-url: https://github.com/nodejs/node/pull/31550
    description: Added `arrayBuffers` to the returned object.
  - version: v7.2.0
    pr-url: https://github.com/nodejs/node/pull/9587
    description: Added `external` to the returned object.
-->

* Returns: {Object}
  * `rss` {integer}
  * `heapTotal` {integer}
  * `heapUsed` {integer}
  * `external` {integer}
  * `arrayBuffers` {integer}

Returns an object describing the memory usage of the Node.js process measured in
bytes.

```mjs
import { memoryUsage } from 'process';

console.log(memoryUsage());
// Prints:
// {
//  rss: 4935680,
//  heapTotal: 1826816,
//  heapUsed: 650472,
//  external: 49879,
//  arrayBuffers: 9386
// }
```

```cjs
const { memoryUsage } = require('process');

console.log(memoryUsage());
// Prints:
// {
//  rss: 4935680,
//  heapTotal: 1826816,
//  heapUsed: 650472,
//  external: 49879,
//  arrayBuffers: 9386
// }
```

* `heapTotal` and `heapUsed` refer to V8's memory usage.
* `external` refers to the memory usage of C++ objects bound to JavaScript
  objects managed by V8.
* `rss`, Resident Set Size, is the amount of space occupied in the main
  memory device (that is a subset of the total allocated memory) for the
  process, including all C++ and JavaScript objects and code.
* `arrayBuffers` refers to memory allocated for `ArrayBuffer`s and
  `SharedArrayBuffer`s, including all Node.js [`Buffer`][]s.
  This is also included in the `external` value. When Node.js is used as an
  embedded library, this value may be `0` because allocations for `ArrayBuffer`s
  may not be tracked in that case.

When using [`Worker`][] threads, `rss` will be a value that is valid for the
entire process, while the other fields will only refer to the current thread.

The `process.memoryUsage()` method iterates over each page to gather
information about memory usage which might be slow depending on the
program memory allocations.

## `process.memoryUsage.rss()`

<!-- YAML
added:
  - v15.6.0
  - v14.18.0
-->

* Returns: {integer}

The `process.memoryUsage.rss()` method returns an integer representing the
Resident Set Size (RSS) in bytes.

The Resident Set Size, is the amount of space occupied in the main
memory device (that is a subset of the total allocated memory) for the
process, including all C++ and JavaScript objects and code.

This is the same value as the `rss` property provided by `process.memoryUsage()`
but `process.memoryUsage.rss()` is faster.

```mjs
import { memoryUsage } from 'process';

console.log(memoryUsage.rss());
// 35655680
```

```cjs
const { rss } = require('process');

console.log(memoryUsage.rss());
// 35655680
```

## `process.nextTick(callback[, ...args])`

<!-- YAML
added: v0.1.26
changes:
  - version: v1.8.1
    pr-url: https://github.com/nodejs/node/pull/1077
    description: Additional arguments after `callback` are now supported.
-->

* `callback` {Function}
* `...args` {any} Additional arguments to pass when invoking the `callback`

`process.nextTick()` adds `callback` to the "next tick queue". This queue is
fully drained after the current operation on the JavaScript stack runs to
completion and before the event loop is allowed to continue. It's possible to
create an infinite loop if one were to recursively call `process.nextTick()`.
See the [Event Loop][] guide for more background.

```mjs
import { nextTick } from 'process';

console.log('start');
nextTick(() => {
  console.log('nextTick callback');
});
console.log('scheduled');
// Output:
// start
// scheduled
// nextTick callback
```

```cjs
const { nextTick } = require('process');

console.log('start');
nextTick(() => {
  console.log('nextTick callback');
});
console.log('scheduled');
// Output:
// start
// scheduled
// nextTick callback
```

This is important when developing APIs in order to give users the opportunity
to assign event handlers _after_ an object has been constructed but before any
I/O has occurred:

```mjs
import { nextTick } from 'process';

function MyThing(options) {
  this.setupOptions(options);

  nextTick(() => {
    this.startDoingStuff();
  });
}

const thing = new MyThing();
thing.getReadyForStuff();

// thing.startDoingStuff() gets called now, not before.
```

```cjs
const { nextTick } = require('process');

function MyThing(options) {
  this.setupOptions(options);

  nextTick(() => {
    this.startDoingStuff();
  });
}

const thing = new MyThing();
thing.getReadyForStuff();

// thing.startDoingStuff() gets called now, not before.
```

It is very important for APIs to be either 100% synchronous or 100%
asynchronous. Consider this example:

```js
// WARNING!  DO NOT USE!  BAD UNSAFE HAZARD!
function maybeSync(arg, cb) {
  if (arg) {
    cb();
    return;
  }

  fs.stat('file', cb);
}
```

This API is hazardous because in the following case:

```js
const maybeTrue = Math.random() > 0.5;

maybeSync(maybeTrue, () => {
  foo();
});

bar();
```

It is not clear whether `foo()` or `bar()` will be called first.

The following approach is much better:

```mjs
import { nextTick } from 'process';

function definitelyAsync(arg, cb) {
  if (arg) {
    nextTick(cb);
    return;
  }

  fs.stat('file', cb);
}
```

```cjs
const { nextTick } = require('process');

function definitelyAsync(arg, cb) {
  if (arg) {
    nextTick(cb);
    return;
  }

  fs.stat('file', cb);
}
```

### When to use `queueMicrotask()` vs. `process.nextTick()`

The [`queueMicrotask()`][] API is an alternative to `process.nextTick()` that
also defers execution of a function using the same microtask queue used to
execute the then, catch, and finally handlers of resolved promises. Within
Node.js, every time the "next tick queue" is drained, the microtask queue
is drained immediately after.

```mjs
import { nextTick } from 'process';

Promise.resolve().then(() => console.log(2));
queueMicrotask(() => console.log(3));
nextTick(() => console.log(1));
// Output:
// 1
// 2
// 3
```

```cjs
const { nextTick } = require('process');

Promise.resolve().then(() => console.log(2));
queueMicrotask(() => console.log(3));
nextTick(() => console.log(1));
// Output:
// 1
// 2
// 3
```

For _most_ userland use cases, the `queueMicrotask()` API provides a portable
and reliable mechanism for deferring execution that works across multiple
JavaScript platform environments and should be favored over `process.nextTick()`.
In simple scenarios, `queueMicrotask()` can be a drop-in replacement for
`process.nextTick()`.

```js
console.log('start');
queueMicrotask(() => {
  console.log('microtask callback');
});
console.log('scheduled');
// Output:
// start
// scheduled
// microtask callback
```

One note-worthy difference between the two APIs is that `process.nextTick()`
allows specifying additional values that will be passed as arguments to the
deferred function when it is called. Achieving the same result with
`queueMicrotask()` requires using either a closure or a bound function:

```js
function deferred(a, b) {
  console.log('microtask', a + b);
}

console.log('start');
queueMicrotask(deferred.bind(undefined, 1, 2));
console.log('scheduled');
// Output:
// start
// scheduled
// microtask 3
```

There are minor differences in the way errors raised from within the next tick
queue and microtask queue are handled. Errors thrown within a queued microtask
callback should be handled within the queued callback when possible. If they are
not, the `process.on('uncaughtException')` event handler can be used to capture
and handle the errors.

When in doubt, unless the specific capabilities of `process.nextTick()` are
needed, use `queueMicrotask()`.

## `process.noDeprecation`

<!-- YAML
added: v0.8.0
-->

* {boolean}

The `process.noDeprecation` property indicates whether the `--no-deprecation`
flag is set on the current Node.js process. See the documentation for
the [`'warning'` event][process_warning] and the
[`emitWarning()` method][process_emit_warning] for more information about this
flag's behavior.

## `process.pid`

<!-- YAML
added: v0.1.15
-->

* {integer}

The `process.pid` property returns the PID of the process.

```mjs
import { pid } from 'process';

console.log(`This process is pid ${pid}`);
```

```cjs
const { pid } = require('process');

console.log(`This process is pid ${pid}`);
```

## `process.platform`

<!-- YAML
added: v0.1.16
-->

* {string}

The `process.platform` property returns a string identifying the operating
system platform on which the Node.js process is running.

Currently possible values are:

* `'aix'`
* `'darwin'`
* `'freebsd'`
* `'linux'`
* `'openbsd'`
* `'sunos'`
* `'win32'`

```mjs
import { platform } from 'process';

console.log(`This platform is ${platform}`);
```

```cjs
const { platform } = require('process');

console.log(`This platform is ${platform}`);
```

The value `'android'` may also be returned if the Node.js is built on the
Android operating system. However, Android support in Node.js
[is experimental][Android building].

## `process.ppid`

<!-- YAML
added:
  - v9.2.0
  - v8.10.0
  - v6.13.0
-->

* {integer}

The `process.ppid` property returns the PID of the parent of the
current process.

```mjs
import { ppid } from 'process';

console.log(`The parent process is pid ${ppid}`);
```

```cjs
const { ppid } = require('process');

console.log(`The parent process is pid ${ppid}`);
```

## `process.release`

<!-- YAML
added: v3.0.0
changes:
  - version: v4.2.0
    pr-url: https://github.com/nodejs/node/pull/3212
    description: The `lts` property is now supported.
-->

* {Object}

The `process.release` property returns an `Object` containing metadata related
to the current release, including URLs for the source tarball and headers-only
tarball.

`process.release` contains the following properties:

* `name` {string} A value that will always be `'node'`.
* `sourceUrl` {string} an absolute URL pointing to a _`.tar.gz`_ file containing
  the source code of the current release.
* `headersUrl`{string} an absolute URL pointing to a _`.tar.gz`_ file containing
  only the source header files for the current release. This file is
  significantly smaller than the full source file and can be used for compiling
  Node.js native add-ons.
* `libUrl` {string} an absolute URL pointing to a _`node.lib`_ file matching the
  architecture and version of the current release. This file is used for
  compiling Node.js native add-ons. _This property is only present on Windows
  builds of Node.js and will be missing on all other platforms._
* `lts` {string} a string label identifying the [LTS][] label for this release.
  This property only exists for LTS releases and is `undefined` for all other
  release types, including _Current_ releases.
  Valid values include the LTS Release code names (including those
  that are no longer supported).
  * `'Dubnium'` for the 10.x LTS line beginning with 10.13.0.
  * `'Erbium'` for the 12.x LTS line beginning with 12.13.0.
    For other LTS Release code names, see [Node.js Changelog Archive](https://github.com/nodejs/node/blob/HEAD/doc/changelogs/CHANGELOG_ARCHIVE.md)

<!-- eslint-skip -->

```js
{
  name: 'node',
  lts: 'Erbium',
  sourceUrl: 'https://nodejs.org/download/release/v12.18.1/node-v12.18.1.tar.gz',
  headersUrl: 'https://nodejs.org/download/release/v12.18.1/node-v12.18.1-headers.tar.gz',
  libUrl: 'https://nodejs.org/download/release/v12.18.1/win-x64/node.lib'
}
```

In custom builds from non-release versions of the source tree, only the
`name` property may be present. The additional properties should not be
relied upon to exist.

## `process.report`

<!-- YAML
added: v11.8.0
changes:
  - version:
     - v13.12.0
     - v12.17.0
    pr-url: https://github.com/nodejs/node/pull/32242
    description: This API is no longer experimental.
-->

* {Object}

`process.report` is an object whose methods are used to generate diagnostic
reports for the current process. Additional documentation is available in the
[report documentation][].

### `process.report.compact`

<!-- YAML
added:
 - v13.12.0
 - v12.17.0
-->

* {boolean}

Write reports in a compact format, single-line JSON, more easily consumable
by log processing systems than the default multi-line format designed for
human consumption.

```mjs
import { report } from 'process';

console.log(`Reports are compact? ${report.compact}`);
```

```cjs
const { report } = require('process');

console.log(`Reports are compact? ${report.compact}`);
```

### `process.report.directory`

<!-- YAML
added: v11.12.0
changes:
  - version:
     - v13.12.0
     - v12.17.0
    pr-url: https://github.com/nodejs/node/pull/32242
    description: This API is no longer experimental.
-->

* {string}

Directory where the report is written. The default value is the empty string,
indicating that reports are written to the current working directory of the
Node.js process.

```mjs
import { report } from 'process';

console.log(`Report directory is ${report.directory}`);
```

```cjs
const { report } = require('process');

console.log(`Report directory is ${report.directory}`);
```

### `process.report.filename`

<!-- YAML
added: v11.12.0
changes:
  - version:
     - v13.12.0
     - v12.17.0
    pr-url: https://github.com/nodejs/node/pull/32242
    description: This API is no longer experimental.
-->

* {string}

Filename where the report is written. If set to the empty string, the output
filename will be comprised of a timestamp, PID, and sequence number. The default
value is the empty string.

```mjs
import { report } from 'process';

console.log(`Report filename is ${report.filename}`);
```

```cjs
const { report } = require('process');

console.log(`Report filename is ${report.filename}`);
```

### `process.report.getReport([err])`

<!-- YAML
added: v11.8.0
changes:
  - version:
     - v13.12.0
     - v12.17.0
    pr-url: https://github.com/nodejs/node/pull/32242
    description: This API is no longer experimental.
-->

* `err` {Error} A custom error used for reporting the JavaScript stack.
* Returns: {Object}

Returns a JavaScript Object representation of a diagnostic report for the
running process. The report's JavaScript stack trace is taken from `err`, if
present.

```mjs
import { report } from 'process';

const data = report.getReport();
console.log(data.header.nodejsVersion);

// Similar to process.report.writeReport()
import fs from 'fs';
fs.writeFileSync('my-report.log', util.inspect(data), 'utf8');
```

```cjs
const { report } = require('process');

const data = report.getReport();
console.log(data.header.nodejsVersion);

// Similar to process.report.writeReport()
const fs = require('fs');
fs.writeFileSync('my-report.log', util.inspect(data), 'utf8');
```

Additional documentation is available in the [report documentation][].

### `process.report.reportOnFatalError`

<!-- YAML
added: v11.12.0
changes:
  - version:
     - v15.0.0
     - v14.17.0
    pr-url: https://github.com/nodejs/node/pull/35654
    description: This API is no longer experimental.
-->

* {boolean}

If `true`, a diagnostic report is generated on fatal errors, such as out of
memory errors or failed C++ assertions.

```mjs
import { report } from 'process';

console.log(`Report on fatal error: ${report.reportOnFatalError}`);
```

```cjs
const { report } = require('process');

console.log(`Report on fatal error: ${report.reportOnFatalError}`);
```

### `process.report.reportOnSignal`

<!-- YAML
added: v11.12.0
changes:
  - version:
     - v13.12.0
     - v12.17.0
    pr-url: https://github.com/nodejs/node/pull/32242
    description: This API is no longer experimental.
-->

* {boolean}

If `true`, a diagnostic report is generated when the process receives the
signal specified by `process.report.signal`.

```mjs
import { report } from 'process';

console.log(`Report on signal: ${report.reportOnSignal}`);
```

```cjs
const { report } = require('process');

console.log(`Report on signal: ${report.reportOnSignal}`);
```

### `process.report.reportOnUncaughtException`

<!-- YAML
added: v11.12.0
changes:
  - version:
     - v13.12.0
     - v12.17.0
    pr-url: https://github.com/nodejs/node/pull/32242
    description: This API is no longer experimental.
-->

* {boolean}

If `true`, a diagnostic report is generated on uncaught exception.

```mjs
import { report } from 'process';

console.log(`Report on exception: ${report.reportOnUncaughtException}`);
```

```cjs
const { report } = require('process');

console.log(`Report on exception: ${report.reportOnUncaughtException}`);
```

### `process.report.signal`

<!-- YAML
added: v11.12.0
changes:
  - version:
     - v13.12.0
     - v12.17.0
    pr-url: https://github.com/nodejs/node/pull/32242
    description: This API is no longer experimental.
-->

* {string}

The signal used to trigger the creation of a diagnostic report. Defaults to
`'SIGUSR2'`.

```mjs
import { report } from 'process';

console.log(`Report signal: ${report.signal}`);
```

```cjs
const { report } = require('process');

console.log(`Report signal: ${report.signal}`);
```

### `process.report.writeReport([filename][, err])`

<!-- YAML
added: v11.8.0
changes:
  - version:
     - v13.12.0
     - v12.17.0
    pr-url: https://github.com/nodejs/node/pull/32242
    description: This API is no longer experimental.
-->

* `filename` {string} Name of the file where the report is written. This
  should be a relative path, that will be appended to the directory specified in
  `process.report.directory`, or the current working directory of the Node.js
  process, if unspecified.

* `err` {Error} A custom error used for reporting the JavaScript stack.

* Returns: {string} Returns the filename of the generated report.

Writes a diagnostic report to a file. If `filename` is not provided, the default
filename includes the date, time, PID, and a sequence number. The report's
JavaScript stack trace is taken from `err`, if present.

```mjs
import { report } from 'process';

report.writeReport();
```

```cjs
const { report } = require('process');

report.writeReport();
```

Additional documentation is available in the [report documentation][].

## `process.resourceUsage()`

<!-- YAML
added: v12.6.0
-->

* Returns: {Object} the resource usage for the current process. All of these
  values come from the `uv_getrusage` call which returns
  a [`uv_rusage_t` struct][uv_rusage_t].
  * `userCPUTime` {integer} maps to `ru_utime` computed in microseconds.
    It is the same value as [`process.cpuUsage().user`][process.cpuUsage].
  * `systemCPUTime` {integer} maps to `ru_stime` computed in microseconds.
    It is the same value as [`process.cpuUsage().system`][process.cpuUsage].
  * `maxRSS` {integer} maps to `ru_maxrss` which is the maximum resident set
    size used in kilobytes.
  * `sharedMemorySize` {integer} maps to `ru_ixrss` but is not supported by
    any platform.
  * `unsharedDataSize` {integer} maps to `ru_idrss` but is not supported by
    any platform.
  * `unsharedStackSize` {integer} maps to `ru_isrss` but is not supported by
    any platform.
  * `minorPageFault` {integer} maps to `ru_minflt` which is the number of
    minor page faults for the process, see
    [this article for more details][wikipedia_minor_fault].
  * `majorPageFault` {integer} maps to `ru_majflt` which is the number of
    major page faults for the process, see
    [this article for more details][wikipedia_major_fault]. This field is not
    supported on Windows.
  * `swappedOut` {integer} maps to `ru_nswap` but is not supported by any
    platform.
  * `fsRead` {integer} maps to `ru_inblock` which is the number of times the
    file system had to perform input.
  * `fsWrite` {integer} maps to `ru_oublock` which is the number of times the
    file system had to perform output.
  * `ipcSent` {integer} maps to `ru_msgsnd` but is not supported by any
    platform.
  * `ipcReceived` {integer} maps to `ru_msgrcv` but is not supported by any
    platform.
  * `signalsCount` {integer} maps to `ru_nsignals` but is not supported by any
    platform.
  * `voluntaryContextSwitches` {integer} maps to `ru_nvcsw` which is the
    number of times a CPU context switch resulted due to a process voluntarily
    giving up the processor before its time slice was completed (usually to
    await availability of a resource). This field is not supported on Windows.
  * `involuntaryContextSwitches` {integer} maps to `ru_nivcsw` which is the
    number of times a CPU context switch resulted due to a higher priority
    process becoming runnable or because the current process exceeded its
    time slice. This field is not supported on Windows.

```mjs
import { resourceUsage } from 'process';

console.log(resourceUsage());
/*
  Will output:
  {
    userCPUTime: 82872,
    systemCPUTime: 4143,
    maxRSS: 33164,
    sharedMemorySize: 0,
    unsharedDataSize: 0,
    unsharedStackSize: 0,
    minorPageFault: 2469,
    majorPageFault: 0,
    swappedOut: 0,
    fsRead: 0,
    fsWrite: 8,
    ipcSent: 0,
    ipcReceived: 0,
    signalsCount: 0,
    voluntaryContextSwitches: 79,
    involuntaryContextSwitches: 1
  }
*/
```

```cjs
const { resourceUsage } = require('process');

console.log(resourceUsage());
/*
  Will output:
  {
    userCPUTime: 82872,
    systemCPUTime: 4143,
    maxRSS: 33164,
    sharedMemorySize: 0,
    unsharedDataSize: 0,
    unsharedStackSize: 0,
    minorPageFault: 2469,
    majorPageFault: 0,
    swappedOut: 0,
    fsRead: 0,
    fsWrite: 8,
    ipcSent: 0,
    ipcReceived: 0,
    signalsCount: 0,
    voluntaryContextSwitches: 79,
    involuntaryContextSwitches: 1
  }
*/
```

## `process.send(message[, sendHandle[, options]][, callback])`

<!-- YAML
added: v0.5.9
-->

* `message` {Object}
* `sendHandle` {net.Server|net.Socket}
* `options` {Object} used to parameterize the sending of certain types of
  handles.`options` supports the following properties:
  * `keepOpen` {boolean} A value that can be used when passing instances of
    `net.Socket`. When `true`, the socket is kept open in the sending process.
    **Default:** `false`.
* `callback` {Function}
* Returns: {boolean}

If Node.js is spawned with an IPC channel, the `process.send()` method can be
used to send messages to the parent process. Messages will be received as a
[`'message'`][] event on the parent's [`ChildProcess`][] object.

If Node.js was not spawned with an IPC channel, `process.send` will be
`undefined`.

The message goes through serialization and parsing. The resulting message might
not be the same as what is originally sent.

## `process.setegid(id)`

<!-- YAML
added: v2.0.0
-->

* `id` {string|number} A group name or ID

The `process.setegid()` method sets the effective group identity of the process.
(See setegid(2).) The `id` can be passed as either a numeric ID or a group
name string. If a group name is specified, this method blocks while resolving
the associated a numeric ID.

```mjs
import process from 'process';

if (process.getegid && process.setegid) {
  console.log(`Current gid: ${process.getegid()}`);
  try {
    process.setegid(501);
    console.log(`New gid: ${process.getegid()}`);
  } catch (err) {
    console.log(`Failed to set gid: ${err}`);
  }
}
```

```cjs
const process = require('process');

if (process.getegid && process.setegid) {
  console.log(`Current gid: ${process.getegid()}`);
  try {
    process.setegid(501);
    console.log(`New gid: ${process.getegid()}`);
  } catch (err) {
    console.log(`Failed to set gid: ${err}`);
  }
}
```

This function is only available on POSIX platforms (i.e. not Windows or
Android).
This feature is not available in [`Worker`][] threads.

## `process.seteuid(id)`

<!-- YAML
added: v2.0.0
-->

* `id` {string|number} A user name or ID

The `process.seteuid()` method sets the effective user identity of the process.
(See seteuid(2).) The `id` can be passed as either a numeric ID or a username
string. If a username is specified, the method blocks while resolving the
associated numeric ID.

```mjs
import process from 'process';

if (process.geteuid && process.seteuid) {
  console.log(`Current uid: ${process.geteuid()}`);
  try {
    process.seteuid(501);
    console.log(`New uid: ${process.geteuid()}`);
  } catch (err) {
    console.log(`Failed to set uid: ${err}`);
  }
}
```

```cjs
const process = require('process');

if (process.geteuid && process.seteuid) {
  console.log(`Current uid: ${process.geteuid()}`);
  try {
    process.seteuid(501);
    console.log(`New uid: ${process.geteuid()}`);
  } catch (err) {
    console.log(`Failed to set uid: ${err}`);
  }
}
```

This function is only available on POSIX platforms (i.e. not Windows or
Android).
This feature is not available in [`Worker`][] threads.

## `process.setgid(id)`

<!-- YAML
added: v0.1.31
-->

* `id` {string|number} The group name or ID

The `process.setgid()` method sets the group identity of the process. (See
setgid(2).) The `id` can be passed as either a numeric ID or a group name
string. If a group name is specified, this method blocks while resolving the
associated numeric ID.

```mjs
import process from 'process';

if (process.getgid && process.setgid) {
  console.log(`Current gid: ${process.getgid()}`);
  try {
    process.setgid(501);
    console.log(`New gid: ${process.getgid()}`);
  } catch (err) {
    console.log(`Failed to set gid: ${err}`);
  }
}
```

```cjs
const process = require('process');

if (process.getgid && process.setgid) {
  console.log(`Current gid: ${process.getgid()}`);
  try {
    process.setgid(501);
    console.log(`New gid: ${process.getgid()}`);
  } catch (err) {
    console.log(`Failed to set gid: ${err}`);
  }
}
```

This function is only available on POSIX platforms (i.e. not Windows or
Android).
This feature is not available in [`Worker`][] threads.

## `process.setgroups(groups)`

<!-- YAML
added: v0.9.4
-->

* `groups` {integer\[]}

The `process.setgroups()` method sets the supplementary group IDs for the
Node.js process. This is a privileged operation that requires the Node.js
process to have `root` or the `CAP_SETGID` capability.

The `groups` array can contain numeric group IDs, group names, or both.

```mjs
import process from 'process';

if (process.getgroups && process.setgroups) {
  try {
    process.setgroups([501]);
    console.log(process.getgroups()); // new groups
  } catch (err) {
    console.log(`Failed to set groups: ${err}`);
  }
}
```

```cjs
const process = require('process');

if (process.getgroups && process.setgroups) {
  try {
    process.setgroups([501]);
    console.log(process.getgroups()); // new groups
  } catch (err) {
    console.log(`Failed to set groups: ${err}`);
  }
}
```

This function is only available on POSIX platforms (i.e. not Windows or
Android).
This feature is not available in [`Worker`][] threads.

## `process.setuid(id)`

<!-- YAML
added: v0.1.28
-->

* `id` {integer | string}

The `process.setuid(id)` method sets the user identity of the process. (See
setuid(2).) The `id` can be passed as either a numeric ID or a username string.
If a username is specified, the method blocks while resolving the associated
numeric ID.

```mjs
import process from 'process';

if (process.getuid && process.setuid) {
  console.log(`Current uid: ${process.getuid()}`);
  try {
    process.setuid(501);
    console.log(`New uid: ${process.getuid()}`);
  } catch (err) {
    console.log(`Failed to set uid: ${err}`);
  }
}
```

```cjs
const process = require('process');

if (process.getuid && process.setuid) {
  console.log(`Current uid: ${process.getuid()}`);
  try {
    process.setuid(501);
    console.log(`New uid: ${process.getuid()}`);
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
added:
  - v16.6.0
  - v14.18.0
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
   * Files: _synchronous_ on Windows and POSIX
   * TTYs (Terminals): _asynchronous_ on Windows, _synchronous_ on POSIX
   * Pipes (and sockets): _synchronous_ on Windows, _asynchronous_ on POSIX

These behaviors are partly for historical reasons, as changing them would
create backward incompatibility, but they are also expected by some users.

Synchronous writes avoid problems such as output written with `console.log()` or
`console.error()` being unexpectedly interleaved, or not written at all if
`process.exit()` is called before an asynchronous write completes. See
[`process.exit()`][] for more information.

_**Warning**_: Synchronous writes block the event loop until the write has
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

[Advanced serialization for `child_process`]: child_process.md#advanced-serialization
[Android building]: https://github.com/nodejs/node/blob/HEAD/BUILDING.md#androidandroid-based-devices-eg-firefox-os
[Child Process]: child_process.md
[Cluster]: cluster.md
[Duplex]: stream.md#duplex-and-transform-streams
[Event Loop]: https://nodejs.org/en/docs/guides/event-loop-timers-and-nexttick/#process-nexttick
[LTS]: https://github.com/nodejs/Release
[Readable]: stream.md#readable-streams
[Signal Events]: #signal-events
[Source Map]: https://sourcemaps.info/spec.html
[Stream compatibility]: stream.md#compatibility-with-older-nodejs-versions
[TTY]: tty.md#tty
[Writable]: stream.md#writable-streams
[`'exit'`]: #event-exit
[`'message'`]: child_process.md#event-message
[`'uncaughtException'`]: #event-uncaughtexception
[`--unhandled-rejections`]: cli.md#--unhandled-rejectionsmode
[`Buffer`]: buffer.md
[`ChildProcess.disconnect()`]: child_process.md#subprocessdisconnect
[`ChildProcess.send()`]: child_process.md#subprocesssendmessage-sendhandle-options-callback
[`ChildProcess`]: child_process.md#class-childprocess
[`Error`]: errors.md#class-error
[`EventEmitter`]: events.md#class-eventemitter
[`NODE_OPTIONS`]: cli.md#node_optionsoptions
[`Promise.race()`]: https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Promise/race
[`Worker`]: worker_threads.md#class-worker
[`Worker` constructor]: worker_threads.md#new-workerfilename-options
[`console.error()`]: console.md#consoleerrordata-args
[`console.log()`]: console.md#consolelogdata-args
[`domain`]: domain.md
[`net.Server`]: net.md#class-netserver
[`net.Socket`]: net.md#class-netsocket
[`os.constants.dlopen`]: os.md#dlopen-constants
[`process.argv`]: #processargv
[`process.config`]: #processconfig
[`process.execPath`]: #processexecpath
[`process.exit()`]: #processexitcode
[`process.exitCode`]: #processexitcode
[`process.hrtime()`]: #processhrtimetime
[`process.hrtime.bigint()`]: #processhrtimebigint
[`process.kill()`]: #processkillpid-signal
[`process.setUncaughtExceptionCaptureCallback()`]: #processsetuncaughtexceptioncapturecallbackfn
[`promise.catch()`]: https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Promise/catch
[`queueMicrotask()`]: globals.md#queuemicrotaskcallback
[`readable.read()`]: stream.md#readablereadsize
[`require()`]: globals.md#require
[`require.main`]: modules.md#accessing-the-main-module
[`subprocess.kill()`]: child_process.md#subprocesskillsignal
[`v8.setFlagsFromString()`]: v8.md#v8setflagsfromstringflags
[debugger]: debugger.md
[deprecation code]: deprecations.md
[note on process I/O]: #a-note-on-process-io
[process.cpuUsage]: #processcpuusagepreviousvalue
[process_emit_warning]: #processemitwarningwarning-type-code-ctor
[process_warning]: #event-warning
[report documentation]: report.md
[terminal raw mode]: tty.md#readstreamsetrawmodemode
[uv_rusage_t]: https://docs.libuv.org/en/v1.x/misc.html#c.uv_rusage_t
[wikipedia_major_fault]: https://en.wikipedia.org/wiki/Page_fault#Major
[wikipedia_minor_fault]: https://en.wikipedia.org/wiki/Page_fault#Minor