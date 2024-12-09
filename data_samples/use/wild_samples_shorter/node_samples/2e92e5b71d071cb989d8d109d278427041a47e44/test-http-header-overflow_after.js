const { createServer, maxHeaderSize } = require('http');
const { createConnection } = require('net');

const CRLF = '\r\n';
const DUMMY_HEADER_NAME = 'Cookie: ';
const DUMMY_HEADER_VALUE = 'a'.repeat(
  // Plus one is to make it 1 byte too big
  maxHeaderSize - DUMMY_HEADER_NAME.length + 2
);
const PAYLOAD_GET = 'GET /blah HTTP/1.1';
const PAYLOAD = PAYLOAD_GET + CRLF +
  DUMMY_HEADER_NAME + DUMMY_HEADER_VALUE + CRLF.repeat(2);
const server = createServer();

server.on('connection', mustCall((socket) => {
  socket.on('error', expectsError({
    name: 'Error',
    message: 'Parse Error: Header overflow',
    code: 'HPE_HEADER_OVERFLOW',
    bytesParsed: maxHeaderSize + PAYLOAD_GET.length + (CRLF.length * 2) + 1,
    rawPacket: Buffer.from(PAYLOAD)
  }));
}));
