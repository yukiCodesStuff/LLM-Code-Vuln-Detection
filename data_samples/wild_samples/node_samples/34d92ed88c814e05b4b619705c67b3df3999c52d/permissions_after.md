      "dependencies": {
        "react": "./app/node_modules/server-side-react/index.js"
      }
    }
  }
}
```

[Import maps][] assume you can get any resource by default. This means
`"dependencies"` at the top level of the policy should be set to `true`.
Policies require this to be opt-in since it enables all resources of the
application cross linkage which doesn't make sense for many scenarios. They also
assume any given scope has access to any scope above its allowed dependencies;
all scopes emulating import maps must set `"cascade": true`.

Import maps only have a single top level scope for their "imports". So for
emulating `"imports"` use the `""` scope. For emulating `"scopes"` use the
`"scopes"` in a similar manner to how `"scopes"` works in import maps.

Caveats: Policies do not use string matching for various finding of scope. They
do URL traversals. This means things like `blob:` and `data:` URLs might not be
entirely interoperable between the two systems. For example import maps can
partially match a `data:` or `blob:` URL by partitioning the URL on a `/`
character, policies intentionally cannot. For `blob:` URLs import map scopes do
not adopt the origin of the `blob:` URL.

Additionally, import maps only work on `import` so it may be desirable to add a
`"import"` condition to all dependency mappings.

## Process-based permissions

### Permission Model

<!-- type=misc -->

> Stability: 1 - Experimental

<!-- name=permission-model -->

The Node.js Permission Model is a mechanism for restricting access to specific
resources during execution.
The API exists behind a flag [`--experimental-permission`][] which when enabled,
will restrict access to all available permissions.

The available permissions are documented by the [`--experimental-permission`][]
flag.

When starting Node.js with `--experimental-permission`,
the ability to access the file system through the `fs` module, spawn processes,
use `node:worker_threads` and enable the runtime inspector
will be restricted.

```console
$ node --experimental-permission index.js
node:internal/modules/cjs/loader:171
  const result = internalModuleStat(filename);
                 ^

Error: Access to this API has been restricted
    at stat (node:internal/modules/cjs/loader:171:18)
    at Module._findPath (node:internal/modules/cjs/loader:627:16)
    at resolveMainPath (node:internal/modules/run_main:19:25)
    at Function.executeUserEntryPoint [as runMain] (node:internal/modules/run_main:76:24)
    at node:internal/main/run_main_module:23:47 {
  code: 'ERR_ACCESS_DENIED',
  permission: 'FileSystemRead'
}