const kInit = Symbol('init');
const kInfoHeaders = Symbol('sent-info-headers');
const kLocalSettings = Symbol('local-settings');
const kNativeFields = Symbol('kNativeFields');
const kOptions = Symbol('options');
const kOwner = owner_symbol;
const kOrigin = Symbol('origin');
const kPendingRequestCalls = Symbol('kPendingRequestCalls');
  paddingBuffer,
  PADDING_BUF_FRAME_LENGTH,
  PADDING_BUF_MAX_PAYLOAD_LENGTH,
  PADDING_BUF_RETURN_VALUE,
  kBitfield,
  kSessionPriorityListenerCount,
  kSessionFrameErrorListenerCount,
  kSessionUint8FieldCount,
  kSessionHasRemoteSettingsListeners,
  kSessionRemoteSettingsIsUpToDate,
  kSessionHasPingListeners,
  kSessionHasAltsvcListeners,
} = binding;

const {
  NGHTTP2_CANCEL,
  }
}

// Keep track of the number/presence of JS event listeners. Knowing that there
// are no listeners allows the C++ code to skip calling into JS for an event.
function sessionListenerAdded(name) {
  switch (name) {
    case 'ping':
      this[kNativeFields][kBitfield] |= 1 << kSessionHasPingListeners;
      break;
    case 'altsvc':
      this[kNativeFields][kBitfield] |= 1 << kSessionHasAltsvcListeners;
      break;
    case 'remoteSettings':
      this[kNativeFields][kBitfield] |= 1 << kSessionHasRemoteSettingsListeners;
      break;
    case 'priority':
      this[kNativeFields][kSessionPriorityListenerCount]++;
      break;
    case 'frameError':
      this[kNativeFields][kSessionFrameErrorListenerCount]++;
      break;
  }
}

function sessionListenerRemoved(name) {
  switch (name) {
    case 'ping':
      if (this.listenerCount(name) > 0) return;
      this[kNativeFields][kBitfield] &= ~(1 << kSessionHasPingListeners);
      break;
    case 'altsvc':
      if (this.listenerCount(name) > 0) return;
      this[kNativeFields][kBitfield] &= ~(1 << kSessionHasAltsvcListeners);
      break;
    case 'remoteSettings':
      if (this.listenerCount(name) > 0) return;
      this[kNativeFields][kBitfield] &=
          ~(1 << kSessionHasRemoteSettingsListeners);
      break;
    case 'priority':
      this[kNativeFields][kSessionPriorityListenerCount]--;
      break;
    case 'frameError':
      this[kNativeFields][kSessionFrameErrorListenerCount]--;
      break;
  }
}

// Also keep track of listeners for the Http2Stream instances, as some events
// are emitted on those objects.
function streamListenerAdded(name) {
  switch (name) {
    case 'priority':
      this[kSession][kNativeFields][kSessionPriorityListenerCount]++;
      break;
    case 'frameError':
      this[kSession][kNativeFields][kSessionFrameErrorListenerCount]++;
      break;
  }
}

function streamListenerRemoved(name) {
  switch (name) {
    case 'priority':
      this[kSession][kNativeFields][kSessionPriorityListenerCount]--;
      break;
    case 'frameError':
      this[kSession][kNativeFields][kSessionFrameErrorListenerCount]--;
      break;
  }
}

function onPing(payload) {
  const session = this[kOwner];
  if (session.destroyed)
    return;
    return;
  session[kUpdateTimer]();
  debugSessionObj(session, 'new settings received');
  session.emit('remoteSettings', session.remoteSettings);
}

// If the stream exists, an attempt will be made to emit an event
  handle.consume(socket._handle);

  this[kHandle] = handle;
  if (this[kNativeFields])
    handle.fields.set(this[kNativeFields]);
  else
    this[kNativeFields] = handle.fields;

  if (socket.encrypted) {
    this[kAlpnProtocol] = socket.alpnProtocol;
    this[kEncrypted] = true;
  session[kProxySocket] = undefined;
  session[kSocket] = undefined;
  session[kHandle] = undefined;
  session[kNativeFields] = new Uint8Array(kSessionUint8FieldCount);
  socket[kSession] = undefined;
  socket[kServer] = undefined;

  // Finally, emit the close and error events (if necessary) on next tick.
    this[kProxySocket] = null;
    this[kSocket] = socket;
    this[kTimeout] = null;
    this[kHandle] = undefined;

    // Do not use nagle's algorithm
    if (typeof socket.setNoDelay === 'function')
      socket.setNoDelay();
      setupFn();
    }

    if (!this[kNativeFields])
      this[kNativeFields] = new Uint8Array(kSessionUint8FieldCount);
    this.on('newListener', sessionListenerAdded);
    this.on('removeListener', sessionListenerRemoved);

    debugSession(type, 'created');
  }

  // Returns undefined if the socket is not yet connected, true if the

  // The settings currently in effect for the remote peer.
  get remoteSettings() {
    if (this[kNativeFields][kBitfield] &
        (1 << kSessionRemoteSettingsIsUpToDate)) {
      const settings = this[kRemoteSettings];
      if (settings !== undefined) {
        return settings;
      }
    }

    if (this.destroyed || this.connecting)
      return {};

    this[kNativeFields][kBitfield] |= (1 << kSessionRemoteSettingsIsUpToDate);
    return this[kRemoteSettings] = getSettings(this[kHandle], true); // Remote
  }

  // Submits a SETTINGS frame to be sent to the remote peer.
  constructor(options, socket, server) {
    super(NGHTTP2_SESSION_SERVER, options, socket);
    this[kServer] = server;
    // This is a bit inaccurate because it does not reflect changes to
    // number of listeners made after the session was created. This should
    // not be an issue in practice. Additionally, the 'priority' event on
    // server instances (or any other object) is fully undocumented.
    this[kNativeFields][kSessionPriorityListenerCount] =
      server.listenerCount('priority');
  }

  get server() {
    return this[kServer];
    this[kProxySocket] = null;

    this.on('pause', streamOnPause);

    this.on('newListener', streamListenerAdded);
    this.on('removeListener', streamListenerRemoved);
  }

  [kUpdateTimer]() {
    if (this.destroyed)
}

function sessionOnError(error) {
  if (this[kServer] !== undefined)
    this[kServer].emit('sessionError', error, this);
}

// When the session times out on the server, try emitting a timeout event.
  const session = new ServerHttp2Session(options, socket, this);

  session.on('stream', sessionOnStream);
  session.on('error', sessionOnError);
  // Don't count our own internal listener.
  session.on('priority', sessionOnPriority);
  session[kNativeFields][kSessionPriorityListenerCount]--;

  if (this.timeout)
    session.setTimeout(this.timeout, sessionOnTimeout);
