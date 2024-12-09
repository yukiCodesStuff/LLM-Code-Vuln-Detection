const kEnableTrace = Symbol('enableTrace');
const kPskCallback = Symbol('pskcallback');
const kPskIdentityHint = Symbol('pskidentityhint');

const noop = () => {};

let ipServernameWarned = false;
function onnewsessionclient(sessionId, session) {
  debug('client emit session');
  const owner = this[owner_symbol];
  owner.emit('session', session);
}

function onnewsession(sessionId, session) {
  debug('onnewsession');
  this.authorized = false;
  this.authorizationError = null;
  this[kRes] = null;

  let wrap;
  if ((socket instanceof net.Socket && socket._handle) || !socket) {
    // 1. connected socket
    this.ssl._secureContext.context = null;
  }
  this.ssl = null;
};

// Constructor guts, arbitrarily factored out.
let warnOnTlsKeylog = true;
    this.emit('secureConnect');
  }

  this.removeListener('end', onConnectEnd);
}

function onConnectEnd() {