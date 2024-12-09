const net = require('net');
const { Duplex } = require('stream');
const tls = require('tls');
const { setImmediate, setTimeout, clearTimeout } = require('timers');

const {
  kIncomingMessage,
  _checkIsHttpToken: checkIsHttpToken
    this.emit('continue');
}

const setTimeoutValue = {
  configurable: true,
  enumerable: true,
  writable: true,
  value: setStreamTimeout
};
ObjectDefineProperty(Http2Stream.prototype, 'setTimeout', setTimeoutValue);
ObjectDefineProperty(Http2Session.prototype, 'setTimeout', setTimeoutValue);


// When the socket emits an error, destroy the associated Http2Session and
// forward it the same error.
    debug('Unknown protocol from %s:%s',
          socket.remoteAddress, socket.remotePort);
    if (!this.emit('unknownProtocol', socket)) {
      debug('Unknown protocol timeout:  %s', options.unknownProtocolTimeout);
      // Install a timeout if the socket was not successfully closed, then
      // destroy the socket to ensure that the underlying resources are
      // released.
      const timer = setTimeout(() => {
        if (!socket.destroyed) {
          debug('UnknownProtocol socket timeout, destroy socket');
          socket.destroy();
        }
      }, options.unknownProtocolTimeout);
      // Un-reference the timer to avoid blocking of application shutdown and
      // clear the timeout if the socket was successfully closed.
      timer.unref();

      socket.once('close', () => clearTimeout(timer));

      // We don't know what to do, so let's just tell the other side what's
      // going on in a format that they *might* understand.
      socket.end('HTTP/1.0 403 Forbidden\r\n' +
                 'Content-Type: text/plain\r\n\r\n' +
    );
  }

  if (options.unknownProtocolTimeout !== undefined)
    validateUint32(options.unknownProtocolTimeout, 'unknownProtocolTimeout');
  else
    // TODO(danbev): is this a good default value?
    options.unknownProtocolTimeout = 10000;


  // Used only with allowHTTP1
  options.Http1IncomingMessage = options.Http1IncomingMessage ||
    http.IncomingMessage;
  options.Http1ServerResponse = options.Http1ServerResponse ||