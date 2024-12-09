const kInit = Symbol('init');
const kInfoHeaders = Symbol('sent-info-headers');
const kLocalSettings = Symbol('local-settings');
const kOptions = Symbol('options');
const kOwner = owner_symbol;
const kOrigin = Symbol('origin');
const kPendingRequestCalls = Symbol('kPendingRequestCalls');
  paddingBuffer,
  PADDING_BUF_FRAME_LENGTH,
  PADDING_BUF_MAX_PAYLOAD_LENGTH,
  PADDING_BUF_RETURN_VALUE
} = binding;

const {
  NGHTTP2_CANCEL,
  }
}

function onPing(payload) {
  const session = this[kOwner];
  if (session.destroyed)
    return;
    return;
  session[kUpdateTimer]();
  debugSessionObj(session, 'new settings received');
  session[kRemoteSettings] = undefined;
  session.emit('remoteSettings', session.remoteSettings);
}

// If the stream exists, an attempt will be made to emit an event
  handle.consume(socket._handle);

  this[kHandle] = handle;

  if (socket.encrypted) {
    this[kAlpnProtocol] = socket.alpnProtocol;
    this[kEncrypted] = true;
  session[kProxySocket] = undefined;
  session[kSocket] = undefined;
  session[kHandle] = undefined;
  socket[kSession] = undefined;
  socket[kServer] = undefined;

  // Finally, emit the close and error events (if necessary) on next tick.
    this[kProxySocket] = null;
    this[kSocket] = socket;
    this[kTimeout] = null;

    // Do not use nagle's algorithm
    if (typeof socket.setNoDelay === 'function')
      socket.setNoDelay();
      setupFn();
    }

    debugSession(type, 'created');
  }

  // Returns undefined if the socket is not yet connected, true if the

  // The settings currently in effect for the remote peer.
  get remoteSettings() {
    const settings = this[kRemoteSettings];
    if (settings !== undefined)
      return settings;

    if (this.destroyed || this.connecting)
      return {};

    return this[kRemoteSettings] = getSettings(this[kHandle], true); // Remote
  }

  // Submits a SETTINGS frame to be sent to the remote peer.
  constructor(options, socket, server) {
    super(NGHTTP2_SESSION_SERVER, options, socket);
    this[kServer] = server;
  }

  get server() {
    return this[kServer];
    this[kProxySocket] = null;

    this.on('pause', streamOnPause);
  }

  [kUpdateTimer]() {
    if (this.destroyed)
}

function sessionOnError(error) {
  if (this[kServer])
    this[kServer].emit('sessionError', error, this);
}

// When the session times out on the server, try emitting a timeout event.
  const session = new ServerHttp2Session(options, socket, this);

  session.on('stream', sessionOnStream);
  session.on('priority', sessionOnPriority);
  session.on('error', sessionOnError);

  if (this.timeout)
    session.setTimeout(this.timeout, sessionOnTimeout);
