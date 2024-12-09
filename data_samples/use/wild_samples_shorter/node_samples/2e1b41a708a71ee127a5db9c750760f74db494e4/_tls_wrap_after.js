const kEnableTrace = Symbol('enableTrace');
const kPskCallback = Symbol('pskcallback');
const kPskIdentityHint = Symbol('pskidentityhint');
const kPendingSession = Symbol('pendingSession');
const kIsVerified = Symbol('verified');

const noop = () => {};

let ipServernameWarned = false;
function onnewsessionclient(sessionId, session) {
  debug('client emit session');
  const owner = this[owner_symbol];
  if (owner[kIsVerified]) {
    owner.emit('session', session);
  } else {
    owner[kPendingSession] = session;
  }
}

function onnewsession(sessionId, session) {
  debug('onnewsession');
  this.authorized = false;
  this.authorizationError = null;
  this[kRes] = null;
  this[kIsVerified] = false;
  this[kPendingSession] = null;

  let wrap;
  if ((socket instanceof net.Socket && socket._handle) || !socket) {
    // 1. connected socket
    this.ssl._secureContext.context = null;
  }
  this.ssl = null;
  this[kPendingSession] = null;
  this[kIsVerified] = false;
};

// Constructor guts, arbitrarily factored out.
let warnOnTlsKeylog = true;
    this.emit('secureConnect');
  }

  this[kIsVerified] = true;
  const session = this[kPendingSession];
  this[kPendingSession] = null;
  if (session)
    this.emit('session', session);

  this.removeListener('end', onConnectEnd);
}

function onConnectEnd() {