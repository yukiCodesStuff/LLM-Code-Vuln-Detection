E('ERR_SOCKET_BAD_PORT', (name, port, allowZero = true) => {
  assert(typeof allowZero === 'boolean',
         "The 'allowZero' argument must be of type boolean.");
  const operator = allowZero ? '>=' : '>';
  return `${name} should be ${operator} 0 and < 65536. Received ${port}.`;
}, RangeError);
E('ERR_SOCKET_BAD_TYPE',
  'Bad socket type specified. Valid types are: udp4, udp6', TypeError);
E('ERR_SOCKET_BUFFER_SIZE',
  'Could not get or set buffer size',
  SystemError);
E('ERR_SOCKET_CLOSED', 'Socket is closed', Error);
E('ERR_SOCKET_DGRAM_IS_CONNECTED', 'Already connected', Error);
E('ERR_SOCKET_DGRAM_NOT_CONNECTED', 'Not connected', Error);
E('ERR_SOCKET_DGRAM_NOT_RUNNING', 'Not running', Error);
E('ERR_SRI_PARSE',
  'Subresource Integrity string %j had an unexpected %j at position %d',
  SyntaxError);
E('ERR_STREAM_ALREADY_FINISHED',
  'Cannot call %s after a stream was finished',
  Error);
E('ERR_STREAM_CANNOT_PIPE', 'Cannot pipe, not readable', Error);
E('ERR_STREAM_DESTROYED', 'Cannot call %s after a stream was destroyed', Error);
E('ERR_STREAM_NULL_VALUES', 'May not write null values to stream', TypeError);
E('ERR_STREAM_PREMATURE_CLOSE', 'Premature close', Error);
E('ERR_STREAM_PUSH_AFTER_EOF', 'stream.push() after EOF', Error);
E('ERR_STREAM_UNSHIFT_AFTER_END_EVENT',
  'stream.unshift() after end event', Error);
E('ERR_STREAM_WRAP', 'Stream has StringDecoder set or is in objectMode', Error);
E('ERR_STREAM_WRITE_AFTER_END', 'write after end', Error);
E('ERR_SYNTHETIC', 'JavaScript Callstack', Error);
E('ERR_SYSTEM_ERROR', 'A system error occurred', SystemError);
E('ERR_TLS_CERT_ALTNAME_FORMAT', 'Invalid subject alternative name string',
  SyntaxError);
E('ERR_TLS_CERT_ALTNAME_INVALID', function(reason, host, cert) {
  this.reason = reason;
  this.host = host;
  this.cert = cert;
  return `Hostname/IP does not match certificate's altnames: ${reason}`;
}, Error);
E('ERR_TLS_DH_PARAM_SIZE', 'DH parameter size %s is less than 2048', Error);
E('ERR_TLS_HANDSHAKE_TIMEOUT', 'TLS handshake timeout', Error);
E('ERR_TLS_INVALID_CONTEXT', '%s must be a SecureContext', TypeError);
E('ERR_TLS_INVALID_PROTOCOL_VERSION',
  '%j is not a valid %s TLS protocol version', TypeError);
E('ERR_TLS_INVALID_STATE', 'TLS socket connection must be securely established',
  Error);
E('ERR_TLS_PROTOCOL_VERSION_CONFLICT',
  'TLS protocol version %j conflicts with secureProtocol %j', TypeError);
E('ERR_TLS_RENEGOTIATION_DISABLED',
  'TLS session renegotiation disabled for this socket', Error);

// This should probably be a `TypeError`.
E('ERR_TLS_REQUIRED_SERVER_NAME',
  '"servername" is required parameter for Server.addContext', Error);
E('ERR_TLS_SESSION_ATTACK', 'TLS session renegotiation attack detected', Error);
E('ERR_TLS_SNI_FROM_SERVER',
  'Cannot issue SNI from a TLS server-side socket', Error);
E('ERR_TRACE_EVENTS_CATEGORY_REQUIRED',
  'At least one category is required', TypeError);
E('ERR_TRACE_EVENTS_UNAVAILABLE', 'Trace events are unavailable', Error);

// This should probably be a `RangeError`.
E('ERR_TTY_INIT_FAILED', 'TTY initialization failed', SystemError);
E('ERR_UNAVAILABLE_DURING_EXIT', 'Cannot call function in process exit ' +
  'handler', Error);
E('ERR_UNCAUGHT_EXCEPTION_CAPTURE_ALREADY_SET',
  '`process.setupUncaughtExceptionCapture()` was called while a capture ' +
    'callback was already active',
  Error);
E('ERR_UNESCAPED_CHARACTERS', '%s contains unescaped characters', TypeError);
E('ERR_UNHANDLED_ERROR',
  // Using a default argument here is important so the argument is not counted
  // towards `Function#length`.
  (err = undefined) => {
    const msg = 'Unhandled error.';
    if (err === undefined) return msg;
    return `${msg} (${err})`;
  }, Error);
E('ERR_UNKNOWN_BUILTIN_MODULE', 'No such built-in module: %s', Error);
E('ERR_UNKNOWN_CREDENTIAL', '%s identifier does not exist: %s', Error);
E('ERR_UNKNOWN_ENCODING', 'Unknown encoding: %s', TypeError);
E('ERR_UNKNOWN_FILE_EXTENSION',
  'Unknown file extension "%s" for %s',
  TypeError);
E('ERR_UNKNOWN_MODULE_FORMAT', 'Unknown module format: %s', RangeError);
E('ERR_UNKNOWN_SIGNAL', 'Unknown signal: %s', TypeError);
E('ERR_UNSUPPORTED_DIR_IMPORT', "Directory import '%s' is not supported " +
'resolving ES modules imported from %s', Error);
E('ERR_UNSUPPORTED_ESM_URL_SCHEME', (url) => {
  let msg = 'Only file and data URLs are supported by the default ESM loader';
  if (isWindows && url.protocol.length === 2) {
    msg +=
      '. On Windows, absolute paths must be valid file:// URLs';
  }
  msg += `. Received protocol '${url.protocol}'`;
  return msg;
}, Error);

// This should probably be a `TypeError`.
E('ERR_VALID_PERFORMANCE_ENTRY_TYPE',
  'At least one valid performance entry type is required', Error);
E('ERR_VM_DYNAMIC_IMPORT_CALLBACK_MISSING',
  'A dynamic import callback was not specified.', TypeError);
E('ERR_VM_MODULE_ALREADY_LINKED', 'Module has already been linked', Error);
E('ERR_VM_MODULE_CANNOT_CREATE_CACHED_DATA',
  'Cached data cannot be created for a module which has been evaluated', Error);
E('ERR_VM_MODULE_DIFFERENT_CONTEXT',
  'Linked modules must use the same context', Error);
E('ERR_VM_MODULE_LINKING_ERRORED',
  'Linking has already failed for the provided module', Error);
E('ERR_VM_MODULE_NOT_MODULE',
  'Provided module is not an instance of Module', Error);
E('ERR_VM_MODULE_STATUS', 'Module status %s', Error);
E('ERR_WASI_ALREADY_STARTED', 'WASI instance has already started', Error);
E('ERR_WORKER_INIT_FAILED', 'Worker initialization failure: %s', Error);
E('ERR_WORKER_INVALID_EXEC_ARGV', (errors, msg = 'invalid execArgv flags') =>
  `Initiated Worker with ${msg}: ${ArrayPrototypeJoin(errors, ', ')}`,
  Error);
E('ERR_WORKER_NOT_RUNNING', 'Worker instance not running', Error);
E('ERR_WORKER_OUT_OF_MEMORY',
  'Worker terminated due to reaching memory limit: %s', Error);
E('ERR_WORKER_PATH', (filename) =>
  'The worker script or module filename must be an absolute path or a ' +
  'relative path starting with \'./\' or \'../\'.' +
  (StringPrototypeStartsWith(filename, 'file://') ?
    ' Wrap file:// URLs with `new URL`.' : ''
  ) +
  (StringPrototypeStartsWith(filename, 'data:text/javascript') ?
    ' Wrap data: URLs with `new URL`.' : ''
  ) +
  ` Received "${filename}"`,
  TypeError);
E('ERR_WORKER_UNSERIALIZABLE_ERROR',
  'Serializing an uncaught exception failed', Error);
E('ERR_WORKER_UNSUPPORTED_OPERATION',
  '%s is not supported in workers', TypeError);
E('ERR_ZLIB_INITIALIZATION_FAILED', 'Initialization failed', Error);