    at node:internal/main/run_main_module:23:47 {
  code: 'ERR_ACCESS_DENIED',
  permission: 'FileSystemRead',
  resource: '/home/user/index.js'
}
```

Allowing access to spawning a process and creating worker threads can be done
using the [`--allow-child-process`][] and [`--allow-worker`][] respectively.

To allow native addons when using permission model, use the [`--allow-addons`][]
flag.

#### Runtime API

When enabling the Permission Model through the [`--experimental-permission`][]
flag a new property `permission` is added to the `process` object.
This property contains one function:

##### `permission.has(scope[, reference])`

API call to check permissions at runtime ([`permission.has()`][])

```js
process.permission.has('fs.write'); // true
process.permission.has('fs.write', '/home/rafaelgss/protected-folder'); // true

process.permission.has('fs.read'); // true
process.permission.has('fs.read', '/home/rafaelgss/protected-folder'); // false
```

#### File System Permissions

To allow access to the file system, use the [`--allow-fs-read`][] and
[`--allow-fs-write`][] flags:

```console
$ node --experimental-permission --allow-fs-read=* --allow-fs-write=* index.js
Hello world!
(node:19836) ExperimentalWarning: Permission is an experimental feature
(Use `node --trace-warnings ...` to show where the warning was created)
```

The valid arguments for both flags are:

* `*` - To allow all `FileSystemRead` or `FileSystemWrite` operations,
  respectively.
* Paths delimited by comma (`,`) to allow only matching `FileSystemRead` or
  `FileSystemWrite` operations, respectively.

Example:

* `--allow-fs-read=*` - It will allow all `FileSystemRead` operations.
* `--allow-fs-write=*` - It will allow all `FileSystemWrite` operations.
* `--allow-fs-write=/tmp/` - It will allow `FileSystemWrite` access to the `/tmp/`
  folder.
* `--allow-fs-read=/tmp/ --allow-fs-read=/home/.gitignore` - It allows `FileSystemRead` access
  to the `/tmp/` folder **and** the `/home/.gitignore` path.

Wildcards are supported too:

* `--allow-fs-read=/home/test*` will allow read access to everything
  that matches the wildcard. e.g: `/home/test/file1` or `/home/test2`

#### Permission Model constraints

There are constraints you need to know before using this system:

* The model does not inherit to a child node process or a worker thread.
* When using the Permission Model the following features will be restricted:
  * Native modules
  * Child process
  * Worker Threads
  * Inspector protocol
  * File system access
* The Permission Model is initialized after the Node.js environment is set up.
  However, certain flags such as `--env-file` or `--openssl-config` are designed
  to read files before environment initialization. As a result, such flags are
  not subject to the rules of the Permission Model.
* OpenSSL engines cannot be requested at runtime when the Permission
  Model is enabled, affecting the built-in crypto, https, and tls modules.

#### Limitations and Known Issues

* When the permission model is enabled, Node.js may resolve some paths
  differently than when it is disabled.
* Relative paths are not supported through the CLI (`--allow-fs-*`).
* Symbolic links will be followed even to locations outside of the set of paths
  that access has been granted to. Relative symbolic links may allow access to
  arbitrary files and directories. When starting applications with the
  permission model enabled, you must ensure that no paths to which access has
  been granted contain relative symbolic links.

[Import maps]: https://url.spec.whatwg.org/#relative-url-with-fragment-string
[Security Policy]: https://github.com/nodejs/node/blob/main/SECURITY.md
[`--allow-addons`]: cli.md#--allow-addons
[`--allow-child-process`]: cli.md#--allow-child-process
[`--allow-fs-read`]: cli.md#--allow-fs-read
[`--allow-fs-write`]: cli.md#--allow-fs-write
[`--allow-worker`]: cli.md#--allow-worker
[`--experimental-permission`]: cli.md#--experimental-permission
[`permission.has()`]: process.md#processpermissionhasscope-reference
[relative-url string]: https://url.spec.whatwg.org/#relative-url-with-fragment-string
[special schemes]: https://url.spec.whatwg.org/#special-scheme