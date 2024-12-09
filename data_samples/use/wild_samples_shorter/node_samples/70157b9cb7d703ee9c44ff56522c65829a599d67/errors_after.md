<a id="ERR_INVALID_URL"></a>
### `ERR_INVALID_URL`

An invalid URL was passed to the [WHATWG][WHATWG URL API] [`URL`
constructor][`new URL(input)`] or the legacy [`url.parse()`][] to be parsed.
The thrown error object typically has an additional property `'input'` that
contains the URL that failed to parse.

<a id="ERR_INVALID_URL_SCHEME"></a>
### `ERR_INVALID_URL_SCHEME`

[`stream.write()`]: stream.md#stream_writable_write_chunk_encoding_callback
[`subprocess.kill()`]: child_process.md#child_process_subprocess_kill_signal
[`subprocess.send()`]: child_process.md#child_process_subprocess_send_message_sendhandle_options_callback
[`url.parse()`]: url.md#url_url_parse_urlstring_parsequerystring_slashesdenotehost
[`util.getSystemErrorName(error.errno)`]: util.md#util_util_getsystemerrorname_err
[`zlib`]: zlib.md
[crypto digest algorithm]: crypto.md#crypto_crypto_gethashes
[define a custom subpath]: packages.md#packages_subpath_exports