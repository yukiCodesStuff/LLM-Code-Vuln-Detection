  if (typeof encoding === 'function') {
    callback = encoding;
    encoding = null;
  }

  var connection = this.connection;
  if (connection &&
      connection._httpMessage === this &&
      connection.writable &&
      !connection.destroyed) {
    // There might be pending data in the this.output buffer.
    var outputLength = this.output.length;
    if (outputLength > 0) {
      var output = this.output;
      var outputEncodings = this.outputEncodings;
      var outputCallbacks = this.outputCallbacks;
      for (var i = 0; i < outputLength; i++) {
        connection.write(output[i], outputEncodings[i],
                         outputCallbacks[i]);
      }

      this.output = [];
      this.outputEncodings = [];
      this.outputCallbacks = [];
    } else if (data.length === 0) {
      if (typeof callback === 'function')
        process.nextTick(callback);
      return true;
    }

    // Directly write to socket.
    return connection.write(data, encoding, callback);
  } else if (connection && connection.destroyed) {
    // The socket was destroyed.  If we're still trying to write to it,
    // then we haven't gotten the 'close' event yet.
    return false;
  } else {
    // buffer, as long as we're not destroyed.
    return this._buffer(data, encoding, callback);
  }
};


OutgoingMessage.prototype._buffer = function(data, encoding, callback) {
  this.output.push(data);
  this.outputEncodings.push(encoding);
  this.outputCallbacks.push(callback);
  return false;
};


OutgoingMessage.prototype._storeHeader = function(firstLine, headers) {
  // firstLine in the case of request is: 'GET /index.html HTTP/1.1\r\n'
  // in the case of response it is: 'HTTP/1.1 200 OK\r\n'
  var state = {
    sentConnectionHeader: false,
    sentContentLengthHeader: false,
    sentTransferEncodingHeader: false,
    sentDateHeader: false,
    sentExpect: false,
    sentTrailer: false,
    messageHeader: firstLine
  };

  if (headers) {
    var keys = Object.keys(headers);
    var isArray = Array.isArray(headers);
    var field, value;

    for (var i = 0, l = keys.length; i < l; i++) {
      var key = keys[i];
      if (isArray) {
        field = headers[key][0];
        value = headers[key][1];
      } else {
        field = key;
        value = headers[key];
      }

      if (Array.isArray(value)) {
        for (var j = 0; j < value.length; j++) {
          storeHeader(this, state, field, value[j]);
        }
      } else {
        storeHeader(this, state, field, value);
      }
    }
  }

  // Date header
  if (this.sendDate === true && state.sentDateHeader === false) {
    state.messageHeader += 'Date: ' + utcDate() + CRLF;
  }

  // Force the connection to close when the response is a 204 No Content or
  // a 304 Not Modified and the user has set a "Transfer-Encoding: chunked"
  // header.
  //
  // RFC 2616 mandates that 204 and 304 responses MUST NOT have a body but
  // node.js used to send out a zero chunk anyway to accommodate clients
  // that don't have special handling for those responses.
  //
  // It was pointed out that this might confuse reverse proxies to the point
  // of creating security liabilities, so suppress the zero chunk and force
  // the connection to close.
  var statusCode = this.statusCode;
  if ((statusCode === 204 || statusCode === 304) &&
      this.chunkedEncoding === true) {
    debug(statusCode + ' response should not use chunked encoding,' +
          ' closing connection.');
    this.chunkedEncoding = false;
    this.shouldKeepAlive = false;
  }

  // keep-alive logic
  if (this._removedHeader.connection) {
    this._last = true;
    this.shouldKeepAlive = false;
  } else if (state.sentConnectionHeader === false) {
    var shouldSendKeepAlive = this.shouldKeepAlive &&
        (state.sentContentLengthHeader ||
         this.useChunkedEncodingByDefault ||
         this.agent);
    if (shouldSendKeepAlive) {
      state.messageHeader += 'Connection: keep-alive\r\n';
    } else {
      this._last = true;
      state.messageHeader += 'Connection: close\r\n';
    }
  }

  if (state.sentContentLengthHeader === false &&
      state.sentTransferEncodingHeader === false) {
    if (!this._hasBody) {
      // Make sure we don't end the 0\r\n\r\n at the end of the message.
      this.chunkedEncoding = false;
    } else if (!this.useChunkedEncodingByDefault) {
      this._last = true;
    } else {
      if (!state.sentTrailer &&
          !this._removedHeader['content-length'] &&
          typeof this._contentLength === 'number') {
        state.messageHeader += 'Content-Length: ' + this._contentLength +
                               '\r\n';
      } else if (!this._removedHeader['transfer-encoding']) {
        state.messageHeader += 'Transfer-Encoding: chunked\r\n';
        this.chunkedEncoding = true;
      } else {
        // We should only be able to get here if both Content-Length and
        // Transfer-Encoding are removed by the user.
        // See: test/parallel/test-http-remove-header-stays-removed.js
        debug('Both Content-Length and Transfer-Encoding are removed');
      }
    }
  }

  this._header = state.messageHeader + CRLF;
  this._headerSent = false;

  // wait until the first body chunk, or close(), is sent to flush,
  // UNLESS we're sending Expect: 100-continue.
  if (state.sentExpect) this._send('');
};

function storeHeader(self, state, field, value) {
  if (!common._checkIsHttpToken(field)) {
    throw new TypeError(
      'Header name must be a valid HTTP Token ["' + field + '"]');
  }
  value = escapeHeaderValue(value);
  state.messageHeader += field + ': ' + value + CRLF;

  if (connectionExpression.test(field)) {
    state.sentConnectionHeader = true;
    if (closeExpression.test(value)) {
      self._last = true;
    } else {
      self.shouldKeepAlive = true;
    }

  } else if (transferEncodingExpression.test(field)) {
    state.sentTransferEncodingHeader = true;
    if (chunkExpression.test(value)) self.chunkedEncoding = true;

  } else if (contentLengthExpression.test(field)) {
    state.sentContentLengthHeader = true;
  } else if (dateExpression.test(field)) {
    state.sentDateHeader = true;
  } else if (expectExpression.test(field)) {
    state.sentExpect = true;
  } else if (trailerExpression.test(field)) {
    state.sentTrailer = true;
  }
}


OutgoingMessage.prototype.setHeader = function(name, value) {
  if (!common._checkIsHttpToken(name))
    throw new TypeError(
      'Header name must be a valid HTTP Token ["' + name + '"]');
  if (typeof name !== 'string')
    throw new TypeError('`name` should be a string in setHeader(name, value).');
  if (value === undefined)
    throw new Error('`value` required in setHeader("' + name + '", value).');
  if (this._header)
    throw new Error('Can\'t set headers after they are sent.');

  if (this._headers === null)
    this._headers = {};

  var key = name.toLowerCase();
  this._headers[key] = value;
  this._headerNames[key] = name;

  if (automaticHeaders[key])
    this._removedHeader[key] = false;
};


OutgoingMessage.prototype.getHeader = function(name) {
  if (arguments.length < 1) {
    throw new Error('`name` is required for getHeader(name).');
  }

  if (!this._headers) return;

  var key = name.toLowerCase();
  return this._headers[key];
};


OutgoingMessage.prototype.removeHeader = function(name) {
  if (arguments.length < 1) {
    throw new Error('`name` is required for removeHeader(name).');
  }

  if (this._header) {
    throw new Error('Can\'t remove headers after they are sent.');
  }

  var key = name.toLowerCase();

  if (key === 'date')
    this.sendDate = false;
  else if (automaticHeaders[key])
    this._removedHeader[key] = true;

  if (this._headers) {
    delete this._headers[key];
    delete this._headerNames[key];
  }
};


OutgoingMessage.prototype._renderHeaders = function() {
  if (this._header) {
    throw new Error('Can\'t render headers after they are sent to the client.');
  }

  var headersMap = this._headers;
  if (!headersMap) return {};

  var headers = {};
  var keys = Object.keys(headersMap);
  var headerNames = this._headerNames;

  for (var i = 0, l = keys.length; i < l; i++) {
    var key = keys[i];
    headers[headerNames[key]] = headersMap[key];
  }
  return headers;
};


Object.defineProperty(OutgoingMessage.prototype, 'headersSent', {
  configurable: true,
  enumerable: true,
  get: function() { return !!this._header; }
});


OutgoingMessage.prototype.write = function(chunk, encoding, callback) {
  if (this.finished) {
    var err = new Error('write after end');
    process.nextTick(writeAfterEndNT, this, err, callback);

    return true;
  }

  if (!this._header) {
    this._implicitHeader();
  }

  if (!this._hasBody) {
    debug('This type of response MUST NOT have a body. ' +
          'Ignoring write() calls.');
    return true;
  }

  if (typeof chunk !== 'string' && !(chunk instanceof Buffer)) {
    throw new TypeError('first argument must be a string or Buffer');
  }


  // If we get an empty string or buffer, then just do nothing, and
  // signal the user to keep writing.
  if (chunk.length === 0) return true;

  var len, ret;
  if (this.chunkedEncoding) {
    if (typeof chunk === 'string' &&
        encoding !== 'hex' &&
        encoding !== 'base64' &&
        encoding !== 'binary') {
      len = Buffer.byteLength(chunk, encoding);
      chunk = len.toString(16) + CRLF + chunk + CRLF;
      ret = this._send(chunk, encoding, callback);
    } else {
      // buffer, or a non-toString-friendly encoding
      if (typeof chunk === 'string')
        len = Buffer.byteLength(chunk, encoding);
      else
        len = chunk.length;

      if (this.connection && !this.connection.corked) {
        this.connection.cork();
        process.nextTick(connectionCorkNT, this.connection);
      }
      this._send(len.toString(16), 'binary', null);
      this._send(crlf_buf, null, null);
      this._send(chunk, encoding, null);
      ret = this._send(crlf_buf, null, callback);
    }
  } else {
    ret = this._send(chunk, encoding, callback);
  }

  debug('write ret = ' + ret);
  return ret;
};


function writeAfterEndNT(self, err, callback) {
  self.emit('error', err);
  if (callback) callback(err);
}


function connectionCorkNT(conn) {
  if (conn)
    conn.uncork();
}


function escapeHeaderValue(value) {
  // Protect against response splitting. The regex test is there to
  // minimize the performance impact in the common case.
  return /[\r\n]/.test(value) ? value.replace(/[\r\n]+[ \t]*/g, '') : value;
}


OutgoingMessage.prototype.addTrailers = function(headers) {
  this._trailer = '';
  var keys = Object.keys(headers);
  var isArray = Array.isArray(headers);
  var field, value;
  for (var i = 0, l = keys.length; i < l; i++) {
    var key = keys[i];
    if (isArray) {
      field = headers[key][0];
      value = headers[key][1];
    } else {
      field = key;
      value = headers[key];
    }
    if (!common._checkIsHttpToken(field)) {
      throw new TypeError(
        'Trailer name must be a valid HTTP Token ["' + field + '"]');
    }
    this._trailer += field + ': ' + escapeHeaderValue(value) + CRLF;
  }
};


const crlf_buf = new Buffer('\r\n');


OutgoingMessage.prototype.end = function(data, encoding, callback) {
  if (typeof data === 'function') {
    callback = data;
    data = null;
  } else if (typeof encoding === 'function') {
    callback = encoding;
    encoding = null;
  }

  if (data && typeof data !== 'string' && !(data instanceof Buffer)) {
    throw new TypeError('first argument must be a string or Buffer');
  }

  if (this.finished) {
    return false;
  }

  var self = this;
  function finish() {
    self.emit('finish');
  }

  if (typeof callback === 'function')
    this.once('finish', callback);

  if (!this._header) {
    if (data) {
      if (typeof data === 'string')
        this._contentLength = Buffer.byteLength(data, encoding);
      else
        this._contentLength = data.length;
    } else {
      this._contentLength = 0;
    }
    this._implicitHeader();
  }

  if (data && !this._hasBody) {
    debug('This type of response MUST NOT have a body. ' +
          'Ignoring data passed to end().');
    data = null;
  }

  if (this.connection && data)
    this.connection.cork();

  var ret;
  if (data) {
    // Normal body write.
    ret = this.write(data, encoding);
  }

  if (this._hasBody && this.chunkedEncoding) {
    ret = this._send('0\r\n' + this._trailer + '\r\n', 'binary', finish);
  } else {
    // Force a flush, HACK.
    ret = this._send('', 'binary', finish);
  }

  if (this.connection && data)
    this.connection.uncork();

  this.finished = true;

  // There is the first message on the outgoing queue, and we've sent
  // everything to the socket.
  debug('outgoing message end.');
  if (this.output.length === 0 &&
      this.connection &&
      this.connection._httpMessage === this) {
    this._finish();
  }

  return ret;
};


OutgoingMessage.prototype._finish = function() {
  assert(this.connection);
  this.emit('prefinish');
};


// This logic is probably a bit confusing. Let me explain a bit:
//
// In both HTTP servers and clients it is possible to queue up several
// outgoing messages. This is easiest to imagine in the case of a client.
// Take the following situation:
//
//    req1 = client.request('GET', '/');
//    req2 = client.request('POST', '/');
//
// When the user does
//
//   req2.write('hello world\n');
//
// it's possible that the first request has not been completely flushed to
// the socket yet. Thus the outgoing messages need to be prepared to queue
// up data internally before sending it on further to the socket's queue.
//
// This function, outgoingFlush(), is called by both the Server and Client
// to attempt to flush any pending messages out to the socket.
OutgoingMessage.prototype._flush = function() {
  var socket = this.socket;
  var outputLength, ret;

  if (socket && socket.writable) {
    // There might be remaining data in this.output; write it out
    outputLength = this.output.length;
    if (outputLength > 0) {
      var output = this.output;
      var outputEncodings = this.outputEncodings;
      var outputCallbacks = this.outputCallbacks;
      for (var i = 0; i < outputLength; i++) {
        ret = socket.write(output[i], outputEncodings[i],
                           outputCallbacks[i]);
      }

      this.output = [];
      this.outputEncodings = [];
      this.outputCallbacks = [];
    }

    if (this.finished) {
      // This is a queue to the server or client to bring in the next this.
      this._finish();
    } else if (ret) {
      // This is necessary to prevent https from breaking
      this.emit('drain');
    }
  }
};


OutgoingMessage.prototype.flushHeaders = function() {
  if (!this._header) {
    this._implicitHeader();
  }

  // Force-flush the headers.
  this._send('');
};

OutgoingMessage.prototype.flush = internalUtil.deprecate(function() {
  this.flushHeaders();
}, 'OutgoingMessage.flush is deprecated. Use flushHeaders instead.');
      for (var i = 0; i < outputLength; i++) {
        connection.write(output[i], outputEncodings[i],
                         outputCallbacks[i]);
      }

      this.output = [];
      this.outputEncodings = [];
      this.outputCallbacks = [];
    } else if (data.length === 0) {
      if (typeof callback === 'function')
        process.nextTick(callback);
      return true;
    }