  _checkInvalidHeaderChar: checkInvalidHeaderChar
} = require('_http_common');
const { OutgoingMessage } = require('_http_outgoing');
const { outHeadersKey, ondrain, nowDate } = require('internal/http');
const {
  defaultTriggerAsyncIdScope,
  getOrSetAsyncId
} = require('internal/async_hooks');
  this.keepAliveTimeout = 5000;
  this._pendingResponseData = 0;
  this.maxHeadersCount = null;
  this.headersTimeout = 40 * 1000; // 40 seconds
}
util.inherits(Server, net.Server);


  var parser = parsers.alloc();
  parser.reinitialize(HTTPParser.REQUEST, parser[is_reused_symbol]);
  parser.socket = socket;

  // We are starting to wait for our headers.
  parser.parsingHeadersStart = nowDate();
  socket.parser = parser;

  // Propagate headers limit from server instance to parser
  if (typeof server.maxHeadersCount === 'number') {

function onParserExecute(server, socket, parser, state, ret) {
  socket._unrefTimer();
  const start = parser.parsingHeadersStart;
  debug('SERVER socketOnParserExecute %d', ret);

  // If we have not parsed the headers, destroy the socket
  // after server.headersTimeout to protect from DoS attacks.
  // start === 0 means that we have parsed headers.
  if (start !== 0 && nowDate() - start > server.headersTimeout) {
    const serverTimeout = server.emit('timeout', socket);

    if (!serverTimeout)
      socket.destroy();
    return;
  }

  onParserExecuteCommon(server, socket, parser, state, ret, undefined);
}

const badRequestResponse = Buffer.from(
function parserOnIncoming(server, socket, state, req, keepAlive) {
  resetSocketTimeout(server, socket, state);

  // Set to zero to communicate that we have finished parsing.
  socket.parser.parsingHeadersStart = 0;

  if (req.upgrade) {
    req.upgrade = req.method === 'CONNECT' ||
                  server.listenerCount('upgrade') > 0;
    if (req.upgrade)