  continueExpression,
  chunkExpression,
  kIncomingMessage,
  HTTPParser,
  isLenient,
  _checkInvalidHeaderChar: checkInvalidHeaderChar,
  prepareError,
  codes
} = require('internal/errors');
const {
  ERR_HTTP_HEADERS_SENT,
  ERR_HTTP_INVALID_STATUS_CODE,
  ERR_HTTP_SOCKET_ENCODING,
  ERR_INVALID_ARG_TYPE,
  DTRACE_HTTP_SERVER_RESPONSE
} = require('internal/dtrace');
const { observerCounts, constants } = internalBinding('performance');
const { NODE_PERFORMANCE_ENTRY_TYPE_HTTP } = constants;

const kServerResponse = Symbol('ServerResponse');
const kServerResponseStatistics = Symbol('ServerResponseStatistics');
  511: 'Network Authentication Required' // RFC 6585 6
};

const kOnExecute = HTTPParser.kOnExecute | 0;
const kOnTimeout = HTTPParser.kOnTimeout | 0;

class HTTPServerAsyncResource {
  this.keepAliveTimeout = 5000;
  this.maxHeadersCount = null;
  this.headersTimeout = 60 * 1000; // 60 seconds
}
ObjectSetPrototypeOf(Server.prototype, net.Server.prototype);
ObjectSetPrototypeOf(Server, net.Server);

  parser[kOnTimeout] =
    onParserTimeout.bind(undefined, server, socket);

  socket._paused = false;
}

function socketSetEncoding() {
  onParserExecuteCommon(server, socket, parser, state, ret, d);
}

function onParserExecute(server, socket, parser, state, ret) {
  // When underlying `net.Socket` instance is consumed - no
  // `data` events are emitted, and thus `socket.setTimeout` fires the
  // callback even if the data is constantly flowing into the socket.
  `HTTP/1.1 400 ${STATUS_CODES[400]}${CRLF}` +
  `Connection: close${CRLF}${CRLF}`, 'ascii'
);
const requestHeaderFieldsTooLargeResponse = Buffer.from(
  `HTTP/1.1 431 ${STATUS_CODES[431]}${CRLF}` +
  `Connection: close${CRLF}${CRLF}`, 'ascii'
);

  if (!this.server.emit('clientError', e, this)) {
    if (this.writable && this.bytesWritten === 0) {
      const response = e.code === 'HPE_HEADER_OVERFLOW' ?
        requestHeaderFieldsTooLargeResponse : badRequestResponse;
      this.write(response);
    }
    this.destroy(e);
  }
      const bodyHead = d.slice(ret, d.length);

      socket.readableFlowing = null;
      server.emit(eventName, req, socket, bodyHead);
    } else {
      // Got CONNECT method, but have no handler.
      socket.destroy();
    }
  }

  if (socket._paused && socket.parser) {
    // onIncoming paused the socket, we should pause the parser as well
  }
}

function resOnFinish(req, res, socket, state, server) {
  // Usually the first incoming element should be our request.  it may
  // be that in the case abortIncoming() was called that the incoming
  // array will be empty.
  if (!req._consuming && !req._readableState.resumeScheduled)
    req._dump();

  res.detachSocket(socket);
  clearIncoming(req);
  process.nextTick(emitCloseNT, res);

      res.end();
    }
  } else {
    server.emit('request', req, res);
  }
  return 0;  // No special treatment.
}