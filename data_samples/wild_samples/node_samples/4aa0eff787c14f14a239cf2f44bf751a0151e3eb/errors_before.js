module.exports = {
  AbortError,
  aggregateTwoErrors,
  aggregateErrors,
  captureLargerStackTrace,
  codes,
  connResetException,
  dnsException,
  // This is exported only to facilitate testing.
  determineSpecificType,
  E,
  errnoException,
  exceptionWithHostPort,
  fatalExceptionStackEnhancers,
  formatList,
  genericNodeError,
  getMessage,
  hideInternalStackFrames,
  hideStackFrames,
  inspectWithNoCustomRetry,
  isErrorStackTraceLimitWritable,
  isStackOverflowError,
  kEnhanceStackBeforeInspector,
  kIsNodeError,
  kNoOverride,
  maybeOverridePrepareStackTrace,
  overrideStackTrace,
  prepareStackTrace,
  setArrowMessage,
  SystemError,
  uvErrmapGet,
  uvException,
  uvExceptionWithHostPort,
};

// To declare an error message, use the E(sym, val, def) function above. The sym
// must be an upper case string. The val can be either a function or a string.
// The def must be an error class.
// The return value of the function must be a string.
// Examples:
// E('EXAMPLE_KEY1', 'This is the error value', Error);
// E('EXAMPLE_KEY2', (a, b) => return `${a} ${b}`, RangeError);
//
// Once an error code has been assigned, the code itself MUST NOT change and
// any given error code must never be reused to identify a different error.
//
// Any error code added here should also be added to the documentation
//
// Note: Please try to keep these in alphabetical order
//
// Note: Node.js specific errors must begin with the prefix ERR_

E('ERR_AMBIGUOUS_ARGUMENT', 'The "%s" argument is ambiguous. %s', TypeError);
E('ERR_ARG_NOT_ITERABLE', '%s must be iterable', TypeError);
E('ERR_ASSERTION', '%s', Error);
E('ERR_ASYNC_CALLBACK', '%s must be a function', TypeError);
E('ERR_ASYNC_TYPE', 'Invalid name for async "type": %s', TypeError);
E('ERR_BROTLI_INVALID_PARAM', '%s is not a valid Brotli parameter', RangeError);
E('ERR_BUFFER_OUT_OF_BOUNDS',
  // Using a default argument here is important so the argument is not counted
  // towards `Function#length`.
  (name = undefined) => {
    if (name) {
      return `"${name}" is outside of buffer bounds`;
    }
    return 'Attempt to access memory outside buffer bounds';
  }, RangeError);
E('ERR_BUFFER_TOO_LARGE',
  'Cannot create a Buffer larger than %s bytes',
  RangeError);
E('ERR_CANNOT_WATCH_SIGINT', 'Cannot watch for SIGINT signals', Error);
E('ERR_CHILD_CLOSED_BEFORE_REPLY',
  'Child closed before reply received', Error);
E('ERR_CHILD_PROCESS_IPC_REQUIRED',
  "Forked processes must have an IPC channel, missing value 'ipc' in %s",
  Error);
E('ERR_CHILD_PROCESS_STDIO_MAXBUFFER', '%s maxBuffer length exceeded',
  RangeError);
E('ERR_CONSOLE_WRITABLE_STREAM',
  'Console expects a writable stream instance for %s', TypeError);
E('ERR_CONTEXT_NOT_INITIALIZED', 'context used is not initialized', Error);
E('ERR_CRYPTO_CUSTOM_ENGINE_NOT_SUPPORTED',
  'Custom engines not supported by this OpenSSL', Error);
E('ERR_CRYPTO_ECDH_INVALID_FORMAT', 'Invalid ECDH format: %s', TypeError);
E('ERR_CRYPTO_ECDH_INVALID_PUBLIC_KEY',
  'Public key is not valid for specified curve', Error);
E('ERR_CRYPTO_ENGINE_UNKNOWN', 'Engine "%s" was not found', Error);
E('ERR_CRYPTO_FIPS_FORCED',
  'Cannot set FIPS mode, it was forced with --force-fips at startup.', Error);
E('ERR_CRYPTO_FIPS_UNAVAILABLE', 'Cannot set FIPS mode in a non-FIPS build.',
  Error);
E('ERR_CRYPTO_HASH_FINALIZED', 'Digest already called', Error);
E('ERR_CRYPTO_HASH_UPDATE_FAILED', 'Hash update failed', Error);
E('ERR_CRYPTO_INCOMPATIBLE_KEY', 'Incompatible %s: %s', Error);
E('ERR_CRYPTO_INCOMPATIBLE_KEY_OPTIONS', 'The selected key encoding %s %s.',
  Error);
E('ERR_CRYPTO_INVALID_DIGEST', 'Invalid digest: %s', TypeError);
E('ERR_CRYPTO_INVALID_JWK', 'Invalid JWK data', TypeError);
E('ERR_CRYPTO_INVALID_KEY_OBJECT_TYPE',
  'Invalid key object type %s, expected %s.', TypeError);
E('ERR_CRYPTO_INVALID_STATE', 'Invalid state for operation %s', Error);
E('ERR_CRYPTO_PBKDF2_ERROR', 'PBKDF2 error', Error);
E('ERR_CRYPTO_SCRYPT_INVALID_PARAMETER', 'Invalid scrypt parameter', Error);
E('ERR_CRYPTO_SCRYPT_NOT_SUPPORTED', 'Scrypt algorithm not supported', Error);
// Switch to TypeError. The current implementation does not seem right.
E('ERR_CRYPTO_SIGN_KEY_REQUIRED', 'No key provided to sign', Error);
E('ERR_DEBUGGER_ERROR', '%s', Error);
E('ERR_DEBUGGER_STARTUP_ERROR', '%s', Error);
E('ERR_DIR_CLOSED', 'Directory handle was closed', Error);
E('ERR_DIR_CONCURRENT_OPERATION',
  'Cannot do synchronous work on directory handle with concurrent ' +
  'asynchronous operations', Error);
E('ERR_DNS_SET_SERVERS_FAILED', 'c-ares failed to set servers: "%s" [%s]',
  Error);
E('ERR_DOMAIN_CALLBACK_NOT_AVAILABLE',
  'A callback was registered through ' +
     'process.setUncaughtExceptionCaptureCallback(), which is mutually ' +
     'exclusive with using the `domain` module',
  Error);
E('ERR_DOMAIN_CANNOT_SET_UNCAUGHT_EXCEPTION_CAPTURE',
  'The `domain` module is in use, which is mutually exclusive with calling ' +
     'process.setUncaughtExceptionCaptureCallback()',
  Error);
E('ERR_DUPLICATE_STARTUP_SNAPSHOT_MAIN_FUNCTION',
  'Deserialize main function is already configured.', Error);
E('ERR_ENCODING_INVALID_ENCODED_DATA', function(encoding, ret) {
  this.errno = ret;
  return `The encoded data was not valid for encoding ${encoding}`;
}, TypeError);
E('ERR_ENCODING_NOT_SUPPORTED', 'The "%s" encoding is not supported',
  RangeError);
E('ERR_EVAL_ESM_CANNOT_PRINT', '--print cannot be used with ESM input', Error);
E('ERR_EVENT_RECURSION', 'The event "%s" is already being dispatched', Error);
E('ERR_FALSY_VALUE_REJECTION', function(reason) {
  this.reason = reason;
  return 'Promise was rejected with falsy value';
}, Error);
E('ERR_FEATURE_UNAVAILABLE_ON_PLATFORM',
  'The feature %s is unavailable on the current platform' +
  ', which is being used to run Node.js',
  TypeError);
E('ERR_FS_CP_DIR_TO_NON_DIR',
  'Cannot overwrite directory with non-directory', SystemError);
E('ERR_FS_CP_EEXIST', 'Target already exists', SystemError);
E('ERR_FS_CP_EINVAL', 'Invalid src or dest', SystemError);
E('ERR_FS_CP_FIFO_PIPE', 'Cannot copy a FIFO pipe', SystemError);
E('ERR_FS_CP_NON_DIR_TO_DIR',
  'Cannot overwrite non-directory with directory', SystemError);
E('ERR_FS_CP_SOCKET', 'Cannot copy a socket file', SystemError);
E('ERR_FS_CP_SYMLINK_TO_SUBDIRECTORY',
  'Cannot overwrite symlink in subdirectory of self', SystemError);
E('ERR_FS_CP_UNKNOWN', 'Cannot copy an unknown file type', SystemError);
E('ERR_FS_EISDIR', 'Path is a directory', SystemError);
E('ERR_FS_FILE_TOO_LARGE', 'File size (%s) is greater than 2 GiB', RangeError);
E('ERR_FS_INVALID_SYMLINK_TYPE',
  'Symlink type must be one of "dir", "file", or "junction". Received "%s"',
  Error); // Switch to TypeError. The current implementation does not seem right
E('ERR_HTTP2_ALTSVC_INVALID_ORIGIN',
  'HTTP/2 ALTSVC frames require a valid origin', TypeError);
E('ERR_HTTP2_ALTSVC_LENGTH',
  'HTTP/2 ALTSVC frames are limited to 16382 bytes', TypeError);
E('ERR_HTTP2_CONNECT_AUTHORITY',
  ':authority header is required for CONNECT requests', Error);
E('ERR_HTTP2_CONNECT_PATH',
  'The :path header is forbidden for CONNECT requests', Error);
E('ERR_HTTP2_CONNECT_SCHEME',
  'The :scheme header is forbidden for CONNECT requests', Error);
E('ERR_HTTP2_GOAWAY_SESSION',
  'New streams cannot be created after receiving a GOAWAY', Error);
E('ERR_HTTP2_HEADERS_AFTER_RESPOND',
  'Cannot specify additional headers after response initiated', Error);
E('ERR_HTTP2_HEADERS_SENT', 'Response has already been initiated.', Error);
E('ERR_HTTP2_HEADER_SINGLE_VALUE',
  'Header field "%s" must only have a single value', TypeError);
E('ERR_HTTP2_INFO_STATUS_NOT_ALLOWED',
  'Informational status codes cannot be used', RangeError);
E('ERR_HTTP2_INVALID_CONNECTION_HEADERS',
  'HTTP/1 Connection specific headers are forbidden: "%s"', TypeError);
E('ERR_HTTP2_INVALID_HEADER_VALUE',
  'Invalid value "%s" for header "%s"', TypeError);
E('ERR_HTTP2_INVALID_INFO_STATUS',
  'Invalid informational status code: %s', RangeError);
E('ERR_HTTP2_INVALID_ORIGIN',
  'HTTP/2 ORIGIN frames require a valid origin', TypeError);
E('ERR_HTTP2_INVALID_PACKED_SETTINGS_LENGTH',
  'Packed settings length must be a multiple of six', RangeError);
E('ERR_HTTP2_INVALID_PSEUDOHEADER',
  '"%s" is an invalid pseudoheader or is used incorrectly', TypeError);
E('ERR_HTTP2_INVALID_SESSION', 'The session has been destroyed', Error);
E('ERR_HTTP2_INVALID_SETTING_VALUE',
  // Using default arguments here is important so the arguments are not counted
  // towards `Function#length`.
  function(name, actual, min = undefined, max = undefined) {
    this.actual = actual;
    if (min !== undefined) {
      this.min = min;
      this.max = max;
    }
    return `Invalid value for setting "${name}": ${actual}`;
  }, TypeError, RangeError);
E('ERR_HTTP2_INVALID_STREAM', 'The stream has been destroyed', Error);
E('ERR_HTTP2_MAX_PENDING_SETTINGS_ACK',
  'Maximum number of pending settings acknowledgements', Error);
E('ERR_HTTP2_NESTED_PUSH',
  'A push stream cannot initiate another push stream.', Error);
E('ERR_HTTP2_NO_MEM', 'Out of memory', Error);
E('ERR_HTTP2_NO_SOCKET_MANIPULATION',
  'HTTP/2 sockets should not be directly manipulated (e.g. read and written)',
  Error);
E('ERR_HTTP2_ORIGIN_LENGTH',
  'HTTP/2 ORIGIN frames are limited to 16382 bytes', TypeError);
E('ERR_HTTP2_OUT_OF_STREAMS',
  'No stream ID is available because maximum stream ID has been reached',
  Error);
E('ERR_HTTP2_PAYLOAD_FORBIDDEN',
  'Responses with %s status must not have a payload', Error);
E('ERR_HTTP2_PING_CANCEL', 'HTTP2 ping cancelled', Error);
E('ERR_HTTP2_PING_LENGTH', 'HTTP2 ping payload must be 8 bytes', RangeError);
E('ERR_HTTP2_PSEUDOHEADER_NOT_ALLOWED',
  'Cannot set HTTP/2 pseudo-headers', TypeError);
E('ERR_HTTP2_PUSH_DISABLED', 'HTTP/2 client has disabled push streams', Error);
E('ERR_HTTP2_SEND_FILE', 'Directories cannot be sent', Error);
E('ERR_HTTP2_SEND_FILE_NOSEEK',
  'Offset or length can only be specified for regular files', Error);
E('ERR_HTTP2_SESSION_ERROR', 'Session closed with error code %s', Error);
E('ERR_HTTP2_SETTINGS_CANCEL', 'HTTP2 session settings canceled', Error);
E('ERR_HTTP2_SOCKET_BOUND',
  'The socket is already bound to an Http2Session', Error);
E('ERR_HTTP2_SOCKET_UNBOUND',
  'The socket has been disconnected from the Http2Session', Error);
E('ERR_HTTP2_STATUS_101',
  'HTTP status code 101 (Switching Protocols) is forbidden in HTTP/2', Error);
E('ERR_HTTP2_STATUS_INVALID', 'Invalid status code: %s', RangeError);
E('ERR_HTTP2_STREAM_CANCEL', function(error) {
  let msg = 'The pending stream has been canceled';
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
E('ERR_HTTP2_TOO_MANY_INVALID_FRAMES', 'Too many invalid HTTP/2 frames', Error);
E('ERR_HTTP2_TRAILERS_ALREADY_SENT',
  'Trailing headers have already been sent', Error);
E('ERR_HTTP2_TRAILERS_NOT_READY',
  'Trailing headers cannot be sent until after the wantTrailers event is ' +
  'emitted', Error);
E('ERR_HTTP2_UNSUPPORTED_PROTOCOL', 'protocol "%s" is unsupported.', Error);
E('ERR_HTTP_BODY_NOT_ALLOWED',
  'Adding content for this request method or response status is not allowed.', Error);
E('ERR_HTTP_CONTENT_LENGTH_MISMATCH',
  'Response body\'s content-length of %s byte(s) does not match the content-length of %s byte(s) set in header', Error);
E('ERR_HTTP_HEADERS_SENT',
  'Cannot %s headers after they are sent to the client', Error);
E('ERR_HTTP_INVALID_HEADER_VALUE',
  'Invalid value "%s" for header "%s"', TypeError);
E('ERR_HTTP_INVALID_STATUS_CODE', 'Invalid status code: %s', RangeError);
E('ERR_HTTP_REQUEST_TIMEOUT', 'Request timeout', Error);
E('ERR_HTTP_SOCKET_ASSIGNED',
  'ServerResponse has an already assigned socket', Error);
E('ERR_HTTP_SOCKET_ENCODING',
  'Changing the socket encoding is not allowed per RFC7230 Section 3.', Error);
E('ERR_HTTP_TRAILER_INVALID',
  'Trailers are invalid with this transfer encoding', Error);
E('ERR_ILLEGAL_CONSTRUCTOR', 'Illegal constructor', TypeError);
E('ERR_IMPORT_ASSERTION_TYPE_FAILED',
  'Module "%s" is not of type "%s"', TypeError);
E('ERR_IMPORT_ASSERTION_TYPE_MISSING',
  'Module "%s" needs an import assertion of type "%s"', TypeError);
E('ERR_IMPORT_ASSERTION_TYPE_UNSUPPORTED',
  'Import assertion type "%s" is unsupported', TypeError);
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
    if (StringPrototypeEndsWith(name, ' argument')) {
      // For cases like 'first argument'
      msg += `${name} `;
    } else {
      const type = StringPrototypeIncludes(name, '.') ? 'property' : 'argument';
      msg += `"${name}" ${type} `;
    }
    msg += 'must be ';

    const types = [];
    const instances = [];
    const other = [];

    for (const value of expected) {
      assert(typeof value === 'string',
             'All expected entries have to be of type string');
      if (ArrayPrototypeIncludes(kTypes, value)) {
        ArrayPrototypePush(types, StringPrototypeToLowerCase(value));
      } else if (RegExpPrototypeExec(classRegExp, value) !== null) {
        ArrayPrototypePush(instances, value);
      } else {
        assert(value !== 'object',
               'The value "object" should be written as "Object"');
        ArrayPrototypePush(other, value);
      }
    }

    // Special handle `object` in case other instances are allowed to outline
    // the differences between each other.
    if (instances.length > 0) {
      const pos = ArrayPrototypeIndexOf(types, 'object');
      if (pos !== -1) {
        ArrayPrototypeSplice(types, pos, 1);
        ArrayPrototypePush(instances, 'Object');
      }
    }

    if (types.length > 0) {
      msg += `${types.length > 1 ? 'one of type' : 'of type'} ${formatList(types, 'or')}`;
      if (instances.length > 0 || other.length > 0)
        msg += ' or ';
    }

    if (instances.length > 0) {
      msg += `an instance of ${formatList(instances, 'or')}`;
      if (other.length > 0)
        msg += ' or ';
    }

    if (other.length > 0) {
      if (other.length > 1) {
        msg += `one of ${formatList(other, 'or')}`;
      } else {
        if (StringPrototypeToLowerCase(other[0]) !== other[0])
          msg += 'an ';
        msg += `${other[0]}`;
      }
    }

    msg += `. Received ${determineSpecificType(actual)}`;

    return msg;
  }, TypeError);
E('ERR_INVALID_ARG_VALUE', (name, value, reason = 'is invalid') => {
  let inspected = lazyInternalUtilInspect().inspect(value);
  if (inspected.length > 128) {
    inspected = `${StringPrototypeSlice(inspected, 0, 128)}...`;
  }
  const type = StringPrototypeIncludes(name, '.') ? 'property' : 'argument';
  return `The ${type} '${name}' ${reason}. Received ${inspected}`;
}, TypeError, RangeError);
E('ERR_INVALID_ASYNC_ID', 'Invalid %s value: %s', RangeError);
E('ERR_INVALID_BUFFER_SIZE',
  'Buffer size must be a multiple of %s', RangeError);
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
E('ERR_INVALID_MIME_SYNTAX', (production, str, invalidIndex) => {
  const msg = invalidIndex !== -1 ? ` at ${invalidIndex}` : '';
  return `The MIME syntax for a ${production} in "${str}" is invalid` + msg;
}, TypeError);
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
  if (value?.constructor?.name) {
    type = `instance of ${value.constructor.name}`;
  } else {
    type = `type ${typeof value}`;
  }
  return `Expected ${input} to be returned for the "${prop}" from the` +
         ` "${name}" function but got ${type}.`;
}, TypeError);
E('ERR_INVALID_RETURN_VALUE', (input, name, value) => {
  const type = determineSpecificType(value);

  return `Expected ${input} to be returned from the "${name}"` +
         ` function but got ${type}.`;
}, TypeError, RangeError);
E('ERR_INVALID_STATE', 'Invalid state: %s', Error, TypeError, RangeError);
E('ERR_INVALID_SYNC_FORK_INPUT',
  'Asynchronous forks do not support ' +
    'Buffer, TypedArray, DataView or string input: %s',
  TypeError);
E('ERR_INVALID_THIS', 'Value of "this" must be of type %s', TypeError);
E('ERR_INVALID_TUPLE', '%s must be an iterable %s tuple', TypeError);
E('ERR_INVALID_URI', 'URI malformed', URIError);
E('ERR_INVALID_URL', function(input) {
  this.input = input;
  // Don't include URL in message.
  // (See https://github.com/nodejs/node/pull/38614)
  return 'Invalid URL';
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
E(
  'ERR_LOADER_CHAIN_INCOMPLETE',
  '"%s" did not call the next hook in its chain and did not' +
  ' explicitly signal a short circuit. If this is intentional, include' +
  ' `shortCircuit: true` in the hook\'s return.',
  Error,
);
E('ERR_MANIFEST_ASSERT_INTEGRITY',
  (moduleURL, realIntegrities) => {
    let msg = `The content of "${
      moduleURL
    }" does not match the expected integrity.`;
    if (realIntegrities.size) {
      const sri = ArrayPrototypeJoin(
        ArrayFrom(realIntegrities.entries(),
                  ({ 0: alg, 1: dgs }) => `${alg}-${dgs}`),
        ' ',
      );
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
E('ERR_MANIFEST_INVALID_SPECIFIER',
  'Manifest resource %s has invalid dependency mapping %s',
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
    args = ArrayPrototypeMap(
      args,
      (a) => (ArrayIsArray(a) ?
        ArrayPrototypeJoin(ArrayPrototypeMap(a, wrap), ' or ') :
        wrap(a)),
    );
    msg += `${formatList(args)} argument${len > 1 ? 's' : ''}`;
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
E('ERR_NETWORK_IMPORT_BAD_RESPONSE',
  "import '%s' received a bad response: %s", Error);
E('ERR_NETWORK_IMPORT_DISALLOWED',
  "import of '%s' by %s is not supported: %s", Error);
E('ERR_NOT_BUILDING_SNAPSHOT',
  'Operation cannot be invoked when not building startup snapshot', Error);
E('ERR_NOT_SUPPORTED_IN_SNAPSHOT',
  '%s is not supported in startup snapshot', Error);
E('ERR_NO_CRYPTO',
  'Node.js is not compiled with OpenSSL crypto support', Error);
E('ERR_NO_ICU',
  '%s is not supported on Node.js compiled without ICU', TypeError);
E('ERR_OPERATION_FAILED', 'Operation failed: %s', Error, TypeError);
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
E('ERR_PARSE_ARGS_INVALID_OPTION_VALUE', '%s', TypeError);
E('ERR_PARSE_ARGS_UNEXPECTED_POSITIONAL', "Unexpected argument '%s'. This " +
  'command does not take positional arguments', TypeError);
E('ERR_PARSE_ARGS_UNKNOWN_OPTION', (option, allowPositionals) => {
  const suggestDashDash = allowPositionals ? '. To specify a positional ' +
    "argument starting with a '-', place it at the end of the command after " +
    `'--', as in '-- ${JSONStringify(option)}` : '';
  return `Unknown option '${option}'${suggestDashDash}`;
}, TypeError);
E('ERR_PERFORMANCE_INVALID_TIMESTAMP',
  '%d is not a valid timestamp', TypeError);
E('ERR_PERFORMANCE_MEASURE_INVALID_OPTIONS', '%s', TypeError);
E('ERR_REQUIRE_ESM',
  function(filename, hasEsmSyntax, parentPath = null, packageJsonPath = null) {
    hideInternalStackFrames(this);
    let msg = `require() of ES Module ${filename}${parentPath ? ` from ${
      parentPath}` : ''} not supported.`;
    if (!packageJsonPath) {
      if (StringPrototypeEndsWith(filename, '.mjs'))
        msg += `\nInstead change the require of ${filename} to a dynamic ` +
            'import() which is available in all CommonJS modules.';
      return msg;
    }
    const path = require('path');
    const basename = parentPath && path.basename(filename) ===
      path.basename(parentPath) ? filename : path.basename(filename);
    if (hasEsmSyntax) {
      msg += `\nInstead change the require of ${basename} in ${parentPath} to` +
        ' a dynamic import() which is available in all CommonJS modules.';
      return msg;
    }
    msg += `\n${basename} is treated as an ES module file as it is a .js ` +
      'file whose nearest parent package.json contains "type": "module" ' +
      'which declares all .js files in that package scope as ES modules.' +
      `\nInstead rename ${basename} to end in .cjs, change the requiring ` +
      'code to use dynamic import() which is available in all CommonJS ' +
      'modules, or change "type": "module" to "type": "commonjs" in ' +
      `${packageJsonPath} to treat all .js files as CommonJS (using .mjs for ` +
      'all ES modules instead).\n';
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
  return `${name} should be ${operator} 0 and < 65536. Received ${determineSpecificType(port)}.`;
}, RangeError);
E('ERR_SOCKET_BAD_TYPE',
  'Bad socket type specified. Valid types are: udp4, udp6', TypeError);
E('ERR_SOCKET_BUFFER_SIZE',
  'Could not get or set buffer size',
  SystemError);
E('ERR_SOCKET_CLOSED', 'Socket is closed', Error);
E('ERR_SOCKET_CLOSED_BEFORE_CONNECTION',
  'Socket closed before the connection was established',
  Error);
E('ERR_SOCKET_CONNECTION_TIMEOUT',
  'Socket connection timeout', Error);
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
E('ERR_TAP_LEXER_ERROR', function(errorMsg) {
  hideInternalStackFrames(this);
  return errorMsg;
}, Error);
E('ERR_TAP_PARSER_ERROR', function(errorMsg, details, tokenCausedError, source) {
  hideInternalStackFrames(this);
  this.cause = tokenCausedError;
  const { column, line, start, end } = tokenCausedError.location;
  const errorDetails = `${details} at line ${line}, column ${column} (start ${start}, end ${end})`;
  return errorMsg + errorDetails;
}, SyntaxError);
E('ERR_TAP_VALIDATION_ERROR', function(errorMsg) {
  hideInternalStackFrames(this);
  return errorMsg;
}, Error);
E('ERR_TEST_FAILURE', function(error, failureType) {
  hideInternalStackFrames(this);
  assert(typeof failureType === 'string' || typeof failureType === 'symbol',
         "The 'failureType' argument must be of type string or symbol.");

  let msg = error?.message ?? error;

  if (typeof msg !== 'string') {
    msg = inspectWithNoCustomRetry(msg);
  }

  this.failureType = failureType;
  this.cause = error;
  return msg;
}, Error);
E('ERR_TLS_ALPN_CALLBACK_INVALID_RESULT', (value, protocols) => {
  return `ALPN callback returned a value (${
    value
  }) that did not match any of the client's offered protocols (${
    protocols.join(', ')
  })`;
}, TypeError);
E('ERR_TLS_ALPN_CALLBACK_WITH_PROTOCOLS',
  'The ALPNCallback and ALPNProtocols TLS options are mutually exclusive',
  TypeError);
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
E('ERR_UNKNOWN_FILE_EXTENSION', (ext, path, suggestion) => {
  let msg = `Unknown file extension "${ext}" for ${path}`;
  if (suggestion) {
    msg += `. ${suggestion}`;
  }
  return msg;
}, TypeError);
E('ERR_UNKNOWN_MODULE_FORMAT', 'Unknown module format: %s for URL %s',
  RangeError);
E('ERR_UNKNOWN_SIGNAL', 'Unknown signal: %s', TypeError);
E('ERR_UNSUPPORTED_DIR_IMPORT', "Directory import '%s' is not supported " +
'resolving ES modules imported from %s', Error);
E('ERR_UNSUPPORTED_ESM_URL_SCHEME', (url, supported) => {
  let msg = `Only URLs with a scheme in: ${formatList(supported)} are supported by the default ESM loader`;
  if (isWindows && url.protocol.length === 2) {
    msg +=
      '. On Windows, absolute paths must be valid file:// URLs';
  }
  msg += `. Received protocol '${url.protocol}'`;
  return msg;
}, Error);
E('ERR_USE_AFTER_CLOSE', '%s was closed', Error);

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
E('ERR_VM_MODULE_LINK_FAILURE', function(message, cause) {
  this.cause = cause;
  return message;
}, Error);
E('ERR_VM_MODULE_NOT_MODULE',
  'Provided module is not an instance of Module', Error);
E('ERR_VM_MODULE_STATUS', 'Module status %s', Error);
E('ERR_WASI_ALREADY_STARTED', 'WASI instance has already started', Error);
E('ERR_WEBASSEMBLY_RESPONSE', 'WebAssembly response %s', TypeError);
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