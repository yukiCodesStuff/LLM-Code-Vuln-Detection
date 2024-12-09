const net = require('net');
const { Duplex } = require('stream');
const tls = require('tls');
const { setImmediate } = require('timers');

const {
  kIncomingMessage,
  _checkIsHttpToken: checkIsHttpToken
    this.emit('continue');
}

const setTimeout = {
  configurable: true,
  enumerable: true,
  writable: true,
  value: setStreamTimeout
};
ObjectDefineProperty(Http2Stream.prototype, 'setTimeout', setTimeout);
ObjectDefineProperty(Http2Session.prototype, 'setTimeout', setTimeout);


// When the socket emits an error, destroy the associated Http2Session and
// forward it the same error.
    debug('Unknown protocol from %s:%s',
          socket.remoteAddress, socket.remotePort);
    if (!this.emit('unknownProtocol', socket)) {
      // We don't know what to do, so let's just tell the other side what's
      // going on in a format that they *might* understand.
      socket.end('HTTP/1.0 403 Forbidden\r\n' +
                 'Content-Type: text/plain\r\n\r\n' +
    );
  }

  // Used only with allowHTTP1
  options.Http1IncomingMessage = options.Http1IncomingMessage ||
    http.IncomingMessage;
  options.Http1ServerResponse = options.Http1ServerResponse ||