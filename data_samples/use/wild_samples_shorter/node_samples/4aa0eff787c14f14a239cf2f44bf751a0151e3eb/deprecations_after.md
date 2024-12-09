
`process.binding()` is for use by Node.js internal code only.

While `process.binding()` has not reached End-of-Life status in general, it is
unavailable when [policies][] are enabled.

### DEP0112: `dgram` private APIs

<!-- YAML
changes:
[from_string_encoding]: buffer.md#static-method-bufferfromstring-encoding
[legacy URL API]: url.md#legacy-url-api
[legacy `urlObject`]: url.md#legacy-urlobject
[policies]: permissions.md#policies
[static methods of `crypto.Certificate()`]: crypto.md#class-certificate
[subpath exports]: packages.md#subpath-exports
[subpath imports]: packages.md#subpath-imports
[subpath patterns]: packages.md#subpath-patterns