function onParserTimeout(server, socket) {
  const serverTimeout = server.emit('timeout', socket);

  if (!serverTimeout)
    socket.destroy();
}

const noop = () => {};
const badRequestResponse = Buffer.from(
  `HTTP/1.1 400 ${STATUS_CODES[400]}\r\n` +
  'Connection: close\r\n\r\n', 'ascii',
);
const requestTimeoutResponse = Buffer.from(
  `HTTP/1.1 408 ${STATUS_CODES[408]}\r\n` +
  'Connection: close\r\n\r\n', 'ascii',
);
const requestHeaderFieldsTooLargeResponse = Buffer.from(
  `HTTP/1.1 431 ${STATUS_CODES[431]}\r\n` +
  'Connection: close\r\n\r\n', 'ascii',
);

function socketOnError(e) {
  // Ignore further errors
  this.removeListener('error', socketOnError);

  if (this.listenerCount('error', noop) === 0) {
    this.on('error', noop);
  }

  if (!this.server.emit('clientError', e, this)) {
    // Caution must be taken to avoid corrupting the remote peer.
    // Reply an error segment if there is no in-flight `ServerResponse`,
    // or no data of the in-flight one has been written yet to this socket.
    if (this.writable &&
        (!this._httpMessage || !this._httpMessage._headerSent)) {
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
}
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
}

function onParserExecuteCommon(server, socket, parser, state, ret, d) {
  resetSocketTimeout(server, socket, state);

  if (ret instanceof Error) {
    prepareError(ret, parser, d);
    debug('parse error', ret);
    socketOnError.call(socket, ret);
  } else if (parser.incoming && parser.incoming.upgrade) {
    // Upgrade or CONNECT
    const req = parser.incoming;
    debug('SERVER upgrade or connect', req.method);

    if (!d)
      d = parser.getCurrentBuffer();

    socket.removeListener('data', state.onData);
    socket.removeListener('end', state.onEnd);
    socket.removeListener('close', state.onClose);
    socket.removeListener('drain', state.onDrain);
    socket.removeListener('error', socketOnError);
    socket.removeListener('timeout', socketOnTimeout);
    unconsume(parser, socket);
    parser.finish();
    freeParser(parser, req, socket);
    parser = null;

    const eventName = req.method === 'CONNECT' ? 'connect' : 'upgrade';
    if (eventName === 'upgrade' || server.listenerCount(eventName) > 0) {
      debug('SERVER have listener for %s', eventName);
      const bodyHead = d.slice(ret, d.length);

      socket.readableFlowing = null;

      server.emit(eventName, req, socket, bodyHead);
    } else {
      // Got CONNECT method, but have no handler.
      socket.destroy();
    }