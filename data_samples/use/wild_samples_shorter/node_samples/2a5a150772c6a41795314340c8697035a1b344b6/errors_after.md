HTTP parsing will abort without a request or response object being created, and
an `Error` with this code will be emitted.

<a id="HPE_CHUNK_EXTENSIONS_OVERFLOW"></a>

### `HPE_CHUNK_EXTENSIONS_OVERFLOW`

<!-- YAML
added: v18.19.1
-->

Too much data was received for a chunk extensions. In order to protect against
malicious or malconfigured clients, if more than 16 KiB of data is received
then an `Error` with this code will be emitted.

<a id="HPE_UNEXPECTED_CONTENT_LENGTH"></a>

### `HPE_UNEXPECTED_CONTENT_LENGTH`
