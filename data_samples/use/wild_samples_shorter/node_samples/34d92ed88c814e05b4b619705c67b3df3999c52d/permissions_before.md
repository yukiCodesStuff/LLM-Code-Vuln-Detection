
When starting Node.js with `--experimental-permission`,
the ability to access the file system through the `fs` module, spawn processes,
and use `node:worker_threads` will be restricted.

```console
$ node --experimental-permission index.js
node:internal/modules/cjs/loader:171