  if (error) {
    this.cause = error;
    if (typeof error.message === 'string')
      msg += ` (caused by: ${error.message})`;
  }
  return msg;
}, Error);
E('ERR_HTTP2_STREAM_ERROR', 'Stream closed with error code %s', Error);
E('ERR_HTTP2_STREAM_SELF_DEPENDENCY',
  'A stream cannot depend on itself', Error);
E('ERR_HTTP2_TRAILERS_ALREADY_SENT',
  'Trailing headers have already been sent', Error);
E('ERR_HTTP2_TRAILERS_NOT_READY',
  'Trailing headers cannot be sent until after the wantTrailers event is ' +
  'emitted', Error);
E('ERR_HTTP2_UNSUPPORTED_PROTOCOL', 'protocol "%s" is unsupported.', Error);
E('ERR_HTTP_HEADERS_SENT',
  'Cannot %s headers after they are sent to the client', Error);
E('ERR_HTTP_INVALID_HEADER_VALUE',
  'Invalid value "%s" for header "%s"', TypeError);
E('ERR_HTTP_INVALID_STATUS_CODE', 'Invalid status code: %s', RangeError);
E('ERR_HTTP_SOCKET_ENCODING',
  'Changing the socket encoding is not allowed per RFC7230 Section 3.', Error);
E('ERR_HTTP_TRAILER_INVALID',
  'Trailers are invalid with this transfer encoding', Error);
E('ERR_INCOMPATIBLE_OPTION_PAIR',
  'Option "%s" cannot be used in combination with option "%s"', TypeError);
E('ERR_INPUT_TYPE_NOT_ALLOWED', '--input-type can only be used with string ' +
  'input via --eval, --print, or STDIN', Error);
E('ERR_INSPECTOR_ALREADY_ACTIVATED',
  'Inspector is already activated. Close it with inspector.close() ' +
  'before activating it again.',
  Error);
E('ERR_INSPECTOR_ALREADY_CONNECTED', '%s is already connected', Error);
E('ERR_INSPECTOR_CLOSED', 'Session was closed', Error);
E('ERR_INSPECTOR_COMMAND', 'Inspector error %d: %s', Error);
E('ERR_INSPECTOR_NOT_ACTIVE', 'Inspector is not active', Error);
E('ERR_INSPECTOR_NOT_AVAILABLE', 'Inspector is not available', Error);
E('ERR_INSPECTOR_NOT_CONNECTED', 'Session is not connected', Error);
E('ERR_INSPECTOR_NOT_WORKER', 'Current thread is not a worker', Error);
E('ERR_INTERNAL_ASSERTION', (message) => {
  const suffix = 'This is caused by either a bug in Node.js ' +
    'or incorrect usage of Node.js internals.\n' +
    'Please open an issue with this stack trace at ' +
    'https://github.com/nodejs/node/issues\n';
  return message === undefined ? suffix : `${message}\n${suffix}`;
}, Error);
E('ERR_INVALID_ADDRESS_FAMILY', function(addressType, host, port) {
  this.host = host;
  this.port = port;
  return `Invalid address family: ${addressType} ${host}:${port}`;
}, RangeError);
E('ERR_INVALID_ARG_TYPE',
  (name, expected, actual) => {
    assert(typeof name === 'string', "'name' must be a string");
    if (!ArrayIsArray(expected)) {
      expected = [expected];
    }

    let msg = 'The ';
    if (name.endsWith(' argument')) {
      // For cases like 'first argument'
      msg += `${name} `;
    } else {
      const type = name.includes('.') ? 'property' : 'argument';
      msg += `"${name}" ${type} `;
    }
    msg += 'must be ';

    const types = [];
    const instances = [];
    const other = [];

    for (const value of expected) {
      assert(typeof value === 'string',
             'All expected entries have to be of type string');
      if (kTypes.includes(value)) {
        types.push(value.toLowerCase());
      } else if (classRegExp.test(value)) {
        instances.push(value);
      } else {
        assert(value !== 'object',
               'The value "object" should be written as "Object"');
        other.push(value);
      }
    }

    // Special handle `object` in case other instances are allowed to outline
    // the differences between each other.
    if (instances.length > 0) {
      const pos = types.indexOf('object');
      if (pos !== -1) {
        types.splice(pos, 1);
        instances.push('Object');
      }
    }

    if (types.length > 0) {
      if (types.length > 2) {
        const last = types.pop();
        msg += `one of type ${types.join(', ')}, or ${last}`;
      } else if (types.length === 2) {
        msg += `one of type ${types[0]} or ${types[1]}`;
      } else {
        msg += `of type ${types[0]}`;
      }
      if (instances.length > 0 || other.length > 0)
        msg += ' or ';
    }

    if (instances.length > 0) {
      if (instances.length > 2) {
        const last = instances.pop();
        msg += `an instance of ${instances.join(', ')}, or ${last}`;
      } else {
        msg += `an instance of ${instances[0]}`;
        if (instances.length === 2) {
          msg += ` or ${instances[1]}`;
        }
      }
      if (other.length > 0)
        msg += ' or ';
    }

    if (other.length > 0) {
      if (other.length > 2) {
        const last = other.pop();
        msg += `one of ${other.join(', ')}, or ${last}`;
      } else if (other.length === 2) {
        msg += `one of ${other[0]} or ${other[1]}`;
      } else {
        if (other[0].toLowerCase() !== other[0])
          msg += 'an ';
        msg += `${other[0]}`;
      }
    }

    if (actual == null) {
      msg += `. Received ${actual}`;
    } else if (typeof actual === 'function' && actual.name) {
      msg += `. Received function ${actual.name}`;
    } else if (typeof actual === 'object') {
      if (actual.constructor && actual.constructor.name) {
        msg += `. Received an instance of ${actual.constructor.name}`;
      } else {
        const inspected = lazyInternalUtilInspect()
          .inspect(actual, { depth: -1 });
        msg += `. Received ${inspected}`;
      }
    } else {
      let inspected = lazyInternalUtilInspect()
        .inspect(actual, { colors: false });
      if (inspected.length > 25)
        inspected = `${inspected.slice(0, 25)}...`;
      msg += `. Received type ${typeof actual} (${inspected})`;
    }
    return msg;
  }, TypeError);
E('ERR_INVALID_ARG_VALUE', (name, value, reason = 'is invalid') => {
  let inspected = lazyInternalUtilInspect().inspect(value);
  if (inspected.length > 128) {
    inspected = `${inspected.slice(0, 128)}...`;
  }
  const type = name.includes('.') ? 'property' : 'argument';
  return `The ${type} '${name}' ${reason}. Received ${inspected}`;
}, TypeError, RangeError);
E('ERR_INVALID_ASYNC_ID', 'Invalid %s value: %s', RangeError);
E('ERR_INVALID_BUFFER_SIZE',
  'Buffer size must be a multiple of %s', RangeError);
E('ERR_INVALID_CALLBACK',
  'Callback must be a function. Received %O', TypeError);
E('ERR_INVALID_CHAR',
  // Using a default argument here is important so the argument is not counted
  // towards `Function#length`.
  (name, field = undefined) => {
    let msg = `Invalid character in ${name}`;
    if (field !== undefined) {
      msg += ` ["${field}"]`;
    }
    return msg;
  }, TypeError);
E('ERR_INVALID_CURSOR_POS',
  'Cannot set cursor row without setting its column', TypeError);
E('ERR_INVALID_FD',
  '"fd" must be a positive integer: %s', RangeError);
E('ERR_INVALID_FD_TYPE', 'Unsupported fd type: %s', TypeError);
E('ERR_INVALID_FILE_URL_HOST',
  'File URL host must be "localhost" or empty on %s', TypeError);
E('ERR_INVALID_FILE_URL_PATH', 'File URL path %s', TypeError);
E('ERR_INVALID_HANDLE_TYPE', 'This handle type cannot be sent', TypeError);
E('ERR_INVALID_HTTP_TOKEN', '%s must be a valid HTTP token ["%s"]', TypeError);
E('ERR_INVALID_IP_ADDRESS', 'Invalid IP address: %s', TypeError);
E('ERR_INVALID_MODULE_SPECIFIER', (request, reason, base = undefined) => {
  return `Invalid module "${request}" ${reason}${base ?
    ` imported from ${base}` : ''}`;
}, TypeError);
E('ERR_INVALID_PACKAGE_CONFIG', (path, base, message) => {
  return `Invalid package config ${path}${base ? ` while importing ${base}` :
    ''}${message ? `. ${message}` : ''}`;
}, Error);
E('ERR_INVALID_PACKAGE_TARGET',
  (pkgPath, key, target, isImport = false, base = undefined) => {
    const relError = typeof target === 'string' && !isImport &&
      target.length && !StringPrototypeStartsWith(target, './');
    if (key === '.') {
      assert(isImport === false);
      return `Invalid "exports" main target ${JSONStringify(target)} defined ` +
        `in the package config ${pkgPath}package.json${base ?
          ` imported from ${base}` : ''}${relError ?
          '; targets must start with "./"' : ''}`;
    }
    return `Invalid "${isImport ? 'imports' : 'exports'}" target ${
      JSONStringify(target)} defined for '${key}' in the package config ${
      pkgPath}package.json${base ? ` imported from ${base}` : ''}${relError ?
      '; targets must start with "./"' : ''}`;
  }, Error);
E('ERR_INVALID_PERFORMANCE_MARK',
  'The "%s" performance mark has not been set', Error);
E('ERR_INVALID_PROTOCOL',
  'Protocol "%s" not supported. Expected "%s"',
  TypeError);
E('ERR_INVALID_REPL_EVAL_CONFIG',
  'Cannot specify both "breakEvalOnSigint" and "eval" for REPL', TypeError);
E('ERR_INVALID_REPL_INPUT', '%s', TypeError);
E('ERR_INVALID_RETURN_PROPERTY', (input, name, prop, value) => {
  return `Expected a valid ${input} to be returned for the "${prop}" from the` +
         ` "${name}" function but got ${value}.`;
}, TypeError);
E('ERR_INVALID_RETURN_PROPERTY_VALUE', (input, name, prop, value) => {
  let type;
  if (value && value.constructor && value.constructor.name) {
    type = `instance of ${value.constructor.name}`;
  } else {
    type = `type ${typeof value}`;
  }
  return `Expected ${input} to be returned for the "${prop}" from the` +
         ` "${name}" function but got ${type}.`;
}, TypeError);
E('ERR_INVALID_RETURN_VALUE', (input, name, value) => {
  let type;
  if (value && value.constructor && value.constructor.name) {
    type = `instance of ${value.constructor.name}`;
  } else {
    type = `type ${typeof value}`;
  }
  return `Expected ${input} to be returned from the "${name}"` +
         ` function but got ${type}.`;
}, TypeError);
E('ERR_INVALID_STATE', 'Invalid state: %s', Error);
E('ERR_INVALID_SYNC_FORK_INPUT',
  'Asynchronous forks do not support ' +
    'Buffer, TypedArray, DataView or string input: %s',
  TypeError);
E('ERR_INVALID_THIS', 'Value of "this" must be of type %s', TypeError);
E('ERR_INVALID_TUPLE', '%s must be an iterable %s tuple', TypeError);
E('ERR_INVALID_URI', 'URI malformed', URIError);
E('ERR_INVALID_URL', function(input) {
  this.input = input;
  return `Invalid URL: ${input}`;
}, TypeError);
E('ERR_INVALID_URL_SCHEME',
  (expected) => {
    if (typeof expected === 'string')
      expected = [expected];
    assert(expected.length <= 2);
    const res = expected.length === 2 ?
      `one of scheme ${expected[0]} or ${expected[1]}` :
      `of scheme ${expected[0]}`;
    return `The URL must be ${res}`;
  }, TypeError);
E('ERR_IPC_CHANNEL_CLOSED', 'Channel closed', Error);
E('ERR_IPC_DISCONNECTED', 'IPC channel is already disconnected', Error);
E('ERR_IPC_ONE_PIPE', 'Child process can have only one IPC pipe', Error);
E('ERR_IPC_SYNC_FORK', 'IPC cannot be used with synchronous forks', Error);
E('ERR_MANIFEST_ASSERT_INTEGRITY',
  (moduleURL, realIntegrities) => {
    let msg = `The content of "${
      moduleURL
    }" does not match the expected integrity.`;
    if (realIntegrities.size) {
      const sri = [...realIntegrities.entries()].map(([alg, dgs]) => {
        return `${alg}-${dgs}`;
      }).join(' ');
      msg += ` Integrities found are: ${sri}`;
    } else {
      msg += ' The resource was not found in the policy.';
    }
    return msg;
  }, Error);
E('ERR_MANIFEST_DEPENDENCY_MISSING',
  'Manifest resource %s does not list %s as a dependency specifier for ' +
  'conditions: %s',
  Error);
E('ERR_MANIFEST_INTEGRITY_MISMATCH',
  'Manifest resource %s has multiple entries but integrity lists do not match',
  SyntaxError);
E('ERR_MANIFEST_INVALID_RESOURCE_FIELD',
  'Manifest resource %s has invalid property value for %s',
  TypeError);
E('ERR_MANIFEST_TDZ', 'Manifest initialization has not yet run', Error);
E('ERR_MANIFEST_UNKNOWN_ONERROR',
  'Manifest specified unknown error behavior "%s".',
  SyntaxError);
E('ERR_METHOD_NOT_IMPLEMENTED', 'The %s method is not implemented', Error);
E('ERR_MISSING_ARGS',
  (...args) => {
    assert(args.length > 0, 'At least one arg needs to be specified');
    let msg = 'The ';
    const len = args.length;
    const wrap = (a) => `"${a}"`;
    args = args.map(
      (a) => (ArrayIsArray(a) ? a.map(wrap).join(' or ') : wrap(a))
    );
    switch (len) {
      case 1:
        msg += `${args[0]} argument`;
        break;
      case 2:
        msg += `${args[0]} and ${args[1]} arguments`;
        break;
      default:
        msg += args.slice(0, len - 1).join(', ');
        msg += `, and ${args[len - 1]} arguments`;
        break;
    }
    return `${msg} must be specified`;
  }, TypeError);
E('ERR_MISSING_OPTION', '%s is required', TypeError);
E('ERR_MODULE_NOT_FOUND', (path, base, type = 'package') => {
  return `Cannot find ${type} '${path}' imported from ${base}`;
}, Error);
E('ERR_MULTIPLE_CALLBACK', 'Callback called multiple times', Error);
E('ERR_NAPI_CONS_FUNCTION', 'Constructor must be a function', TypeError);
E('ERR_NAPI_INVALID_DATAVIEW_ARGS',
  'byte_offset + byte_length should be less than or equal to the size in ' +
    'bytes of the array passed in',
  RangeError);
E('ERR_NAPI_INVALID_TYPEDARRAY_ALIGNMENT',
  'start offset of %s should be a multiple of %s', RangeError);
E('ERR_NAPI_INVALID_TYPEDARRAY_LENGTH',
  'Invalid typed array length', RangeError);
E('ERR_NO_CRYPTO',
  'Node.js is not compiled with OpenSSL crypto support', Error);
E('ERR_NO_ICU',
  '%s is not supported on Node.js compiled without ICU', TypeError);
E('ERR_OPERATION_FAILED', 'Operation failed: %s', Error);
E('ERR_OUT_OF_RANGE',
  (str, range, input, replaceDefaultBoolean = false) => {
    assert(range, 'Missing "range" argument');
    let msg = replaceDefaultBoolean ? str :
      `The value of "${str}" is out of range.`;
    let received;
    if (NumberIsInteger(input) && MathAbs(input) > 2 ** 32) {
      received = addNumericalSeparator(String(input));
    } else if (typeof input === 'bigint') {
      received = String(input);
      if (input > 2n ** 32n || input < -(2n ** 32n)) {
        received = addNumericalSeparator(received);
      }
      received += 'n';
    } else {
      received = lazyInternalUtilInspect().inspect(input);
    }
    msg += ` It must be ${range}. Received ${received}`;
    return msg;
  }, RangeError);
E('ERR_PACKAGE_IMPORT_NOT_DEFINED', (specifier, packagePath, base) => {
  return `Package import specifier "${specifier}" is not defined${packagePath ?
    ` in package ${packagePath}package.json` : ''} imported from ${base}`;
}, TypeError);
E('ERR_PACKAGE_PATH_NOT_EXPORTED', (pkgPath, subpath, base = undefined) => {
  if (subpath === '.')
    return `No "exports" main defined in ${pkgPath}package.json${base ?
      ` imported from ${base}` : ''}`;
  return `Package subpath '${subpath}' is not defined by "exports" in ${
    pkgPath}package.json${base ? ` imported from ${base}` : ''}`;
}, Error);
E('ERR_QUIC_FAILED_TO_CREATE_SESSION', 'Failed to create QuicSession', Error);
E('ERR_QUIC_INVALID_REMOTE_TRANSPORT_PARAMS',
  'Invalid remote transport params', Error);
E('ERR_QUIC_INVALID_TLS_SESSION_TICKET',
  'Invalid TLS session ticket', Error);
E('ERR_QUIC_VERSION_NEGOTIATION',
  (version, requestedVersions, supportedVersions) => {
    return 'QUIC session received version negotiation from server. ' +
      `Version: ${version}. Requested: ${requestedVersions.join(', ')} ` +
      `Supported: ${supportedVersions.join(', ')}`;
  },
  Error);
E('ERR_REQUIRE_ESM',
  (filename, parentPath = null, packageJsonPath = null) => {
    let msg = `Must use import to load ES Module: ${filename}`;
    if (parentPath && packageJsonPath) {
      const path = require('path');
      const basename = path.basename(filename) === path.basename(parentPath) ?
        filename : path.basename(filename);
      msg +=
        '\nrequire() of ES modules is not supported.\nrequire() of ' +
        `${filename} from ${parentPath} ` +
        'is an ES module file as it is a .js file whose nearest parent ' +
        'package.json contains "type": "module" which defines all .js ' +
        'files in that package scope as ES modules.\nInstead rename ' +
        `${basename} to end in .cjs, change the requiring code to use ` +
        'import(), or remove "type": "module" from ' +
        `${packageJsonPath}.\n`;
      return msg;
    }
    return msg;
  }, Error);
E('ERR_SCRIPT_EXECUTION_INTERRUPTED',
  'Script execution was interrupted by `SIGINT`', Error);
E('ERR_SERVER_ALREADY_LISTEN',
  'Listen method has been called more than once without closing.', Error);
E('ERR_SERVER_NOT_RUNNING', 'Server is not running.', Error);
E('ERR_SOCKET_ALREADY_BOUND', 'Socket is already bound', Error);
E('ERR_SOCKET_BAD_BUFFER_SIZE',
  'Buffer size must be a positive integer', TypeError);
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
E('ERR_TLS_CERT_ALTNAME_INVALID', function(reason, host, cert) {
  this.reason = reason;
  this.host = host;
  this.cert = cert;
  return `Hostname/IP does not match certificate's altnames: ${reason}`;
}, Error);
E('ERR_TLS_DH_PARAM_SIZE', 'DH parameter size %s is less than 2048', Error);
E('ERR_TLS_HANDSHAKE_TIMEOUT', 'TLS handshake timeout', Error);
E('ERR_TLS_INVALID_CONTEXT', '%s must be a SecureContext', TypeError),
E('ERR_TLS_INVALID_STATE', 'TLS socket connection must be securely established',
  Error),
E('ERR_TLS_INVALID_PROTOCOL_VERSION',
  '%j is not a valid %s TLS protocol version', TypeError);
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
  `Initiated Worker with ${msg}: ${errors.join(', ')}`,
  Error);
E('ERR_WORKER_NOT_RUNNING', 'Worker instance not running', Error);
E('ERR_WORKER_OUT_OF_MEMORY',
  'Worker terminated due to reaching memory limit: %s', Error);
E('ERR_WORKER_PATH', (filename) =>
  'The worker script or module filename must be an absolute path or a ' +
  'relative path starting with \'./\' or \'../\'.' +
  (filename.startsWith('file://') ?
    ' Wrap file:// URLs with `new URL`.' : ''
  ) +
  (filename.startsWith('data:text/javascript') ?
    ' Wrap data: URLs with `new URL`.' : ''
  ) +
  ` Received "${filename}"`,
  TypeError);
E('ERR_WORKER_UNSERIALIZABLE_ERROR',
  'Serializing an uncaught exception failed', Error);
E('ERR_WORKER_UNSUPPORTED_EXTENSION',
  'The worker script extension must be ".js", ".mjs", or ".cjs". Received "%s"',
  TypeError);
E('ERR_WORKER_UNSUPPORTED_OPERATION',
  '%s is not supported in workers', TypeError);
E('ERR_ZLIB_INITIALIZATION_FAILED', 'Initialization failed', Error);