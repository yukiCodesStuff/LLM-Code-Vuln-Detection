  _checkInvalidHeaderChar: checkInvalidHeaderChar
} = require('_http_common');
const { OutgoingMessage } = require('_http_outgoing');
const { outHeadersKey, ondrain } = require('internal/http');
const {
  defaultTriggerAsyncIdScope,
  getOrSetAsyncId
} = require('internal/async_hooks');
  this.keepAliveTimeout = 5000;
  this._pendingResponseData = 0;
  this.maxHeadersCount = null;
}
util.inherits(Server, net.Server);


  var parser = parsers.alloc();
  parser.reinitialize(HTTPParser.REQUEST, parser[is_reused_symbol]);
  parser.socket = socket;
  socket.parser = parser;

  // Propagate headers limit from server instance to parser
  if (typeof server.maxHeadersCount === 'number') {

function onParserExecute(server, socket, parser, state, ret) {
  socket._unrefTimer();
  debug('SERVER socketOnParserExecute %d', ret);
  onParserExecuteCommon(server, socket, parser, state, ret, undefined);
}

const badRequestResponse = Buffer.from(
function parserOnIncoming(server, socket, state, req, keepAlive) {
  resetSocketTimeout(server, socket, state);

  if (req.upgrade) {
    req.upgrade = req.method === 'CONNECT' ||
                  server.listenerCount('upgrade') > 0;
    if (req.upgrade)