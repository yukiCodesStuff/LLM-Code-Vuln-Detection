  continueExpression,
  chunkExpression,
  kIncomingMessage,
  kRequestTimeout,
  HTTPParser,
  isLenient,
  _checkInvalidHeaderChar: checkInvalidHeaderChar,
  prepareError,
  codes
} = require('internal/errors');
const {
  ERR_HTTP_REQUEST_TIMEOUT,
  ERR_HTTP_HEADERS_SENT,
  ERR_HTTP_INVALID_STATUS_CODE,
  ERR_HTTP_SOCKET_ENCODING,
  ERR_INVALID_ARG_TYPE,
  DTRACE_HTTP_SERVER_RESPONSE
} = require('internal/dtrace');
const { observerCounts, constants } = internalBinding('performance');
const { setTimeout, clearTimeout } = require('timers');
const { NODE_PERFORMANCE_ENTRY_TYPE_HTTP } = constants;

const kServerResponse = Symbol('ServerResponse');
const kServerResponseStatistics = Symbol('ServerResponseStatistics');
  511: 'Network Authentication Required' // RFC 6585 6
};

const kOnMessageBegin = HTTPParser.kOnMessageBegin | 0;
const kOnExecute = HTTPParser.kOnExecute | 0;
const kOnTimeout = HTTPParser.kOnTimeout | 0;

class HTTPServerAsyncResource {
  this.keepAliveTimeout = 5000;
  this.maxHeadersCount = null;
  this.headersTimeout = 60 * 1000; // 60 seconds
  this.requestTimeout = 0; // 120 seconds
}
ObjectSetPrototypeOf(Server.prototype, net.Server.prototype);
ObjectSetPrototypeOf(Server, net.Server);

  parser[kOnTimeout] =
    onParserTimeout.bind(undefined, server, socket);

  // When receiving new requests on the same socket (pipelining or keep alive)
  // make sure the requestTimeout is active.
  parser[kOnMessageBegin] =
    setRequestTimeout.bind(undefined, server, socket);

  // This protects from DOS attack where an attacker establish the connection
  // without sending any data on applications where server.timeout is left to
  // the default value of zero.
  setRequestTimeout(server, socket);

  socket._paused = false;
}

function socketSetEncoding() {
  onParserExecuteCommon(server, socket, parser, state, ret, d);
}

function onRequestTimeout(socket) {
  socket[kRequestTimeout] = undefined;
  socketOnError.call(socket, new ERR_HTTP_REQUEST_TIMEOUT());
}

function onParserExecute(server, socket, parser, state, ret) {
  // When underlying `net.Socket` instance is consumed - no
  // `data` events are emitted, and thus `socket.setTimeout` fires the
  // callback even if the data is constantly flowing into the socket.
  `HTTP/1.1 400 ${STATUS_CODES[400]}${CRLF}` +
  `Connection: close${CRLF}${CRLF}`, 'ascii'
);
const requestTimeoutResponse = Buffer.from(
  `HTTP/1.1 408 ${STATUS_CODES[408]}${CRLF}` +
  `Connection: close${CRLF}${CRLF}`, 'ascii'
);
const requestHeaderFieldsTooLargeResponse = Buffer.from(
  `HTTP/1.1 431 ${STATUS_CODES[431]}${CRLF}` +
  `Connection: close${CRLF}${CRLF}`, 'ascii'
);

  if (!this.server.emit('clientError', e, this)) {
    if (this.writable && this.bytesWritten === 0) {
      let response;

      switch (e.code) {
        case 'HPE_HEADER_OVERFLOW':
          response = requestHeaderFieldsTooLargeResponse;
          break;
        case 'ERR_HTTP_REQUEST_TIMEOUT':
          response = requestTimeoutResponse;
          break;
        default:
          response = badRequestResponse;
          break;
      }

      this.write(response);
    }
    this.destroy(e);
  }
      const bodyHead = d.slice(ret, d.length);

      socket.readableFlowing = null;

      // Clear the requestTimeout after upgrading the connection.
      clearRequestTimeout(req);

      server.emit(eventName, req, socket, bodyHead);
    } else {
      // Got CONNECT method, but have no handler.
      socket.destroy();
    }
  } else {
    // When receiving new requests on the same socket (pipelining or keep alive)
    // make sure the requestTimeout is active.
    parser[kOnMessageBegin] =
      setRequestTimeout.bind(undefined, server, socket);
  }

  if (socket._paused && socket.parser) {
    // onIncoming paused the socket, we should pause the parser as well
  }
}

function setRequestTimeout(server, socket) {
  // Set the request timeout handler.
  if (
    !socket[kRequestTimeout] &&
    server.requestTimeout && server.requestTimeout > 0
  ) {
    debug('requestTimeout timer set');
    socket[kRequestTimeout] =
      setTimeout(onRequestTimeout, server.requestTimeout, socket).unref();
  }
}

function clearRequestTimeout(req) {
  if (!req) {
    req = this;
  }

  if (!req[kRequestTimeout]) {
    return;
  }

  debug('requestTimeout timer cleared');
  clearTimeout(req[kRequestTimeout]);
  req[kRequestTimeout] = undefined;
}

function resOnFinish(req, res, socket, state, server) {
  // Usually the first incoming element should be our request.  it may
  // be that in the case abortIncoming() was called that the incoming
  // array will be empty.
  if (!req._consuming && !req._readableState.resumeScheduled)
    req._dump();

  // Make sure the requestTimeout is cleared before finishing.
  // This might occur if the application has sent a response
  // without consuming the request body, which would have alredy
  // cleared the timer.
  // clearRequestTimeout can be executed even if the timer is not active,
  // so this is safe.
  clearRequestTimeout(req);

  res.detachSocket(socket);
  clearIncoming(req);
  process.nextTick(emitCloseNT, res);

      res.end();
    }
  } else {
    req.on('end', clearRequestTimeout);

    server.emit('request', req, res);
  }
  return 0;  // No special treatment.
}