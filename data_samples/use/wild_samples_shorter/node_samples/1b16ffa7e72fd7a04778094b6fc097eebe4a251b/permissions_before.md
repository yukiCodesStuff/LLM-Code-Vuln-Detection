There are constraints you need to know before using this system:

* Native modules are restricted by default when using the Permission Model.
* Relative paths are not supported through the CLI (`--allow-fs-*`).
* The model does not inherit to a child node process.
* The model does not inherit to a worker thread.
* When creating symlinks the target (first argument) should have read and