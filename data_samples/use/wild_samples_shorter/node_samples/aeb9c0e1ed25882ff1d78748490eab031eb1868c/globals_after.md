#### Static method: `AbortSignal.timeout(delay)`

<!-- YAML
added: v17.3.0
-->

* `delay` {number} The number of milliseconds to wait before triggering
  the AbortSignal.
#### `abortSignal.throwIfAborted()`

<!-- YAML
added: v17.3.0
-->

If `abortSignal.aborted` is `true`, throws `abortSignal.reason`.
