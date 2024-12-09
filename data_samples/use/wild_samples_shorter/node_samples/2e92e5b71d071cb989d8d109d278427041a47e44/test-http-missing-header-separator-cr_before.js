const http = require('http');
const net = require('net');

const msg = [
  'GET / HTTP/1.1',
  'Host: localhost',
  'Dummy: x\nContent-Length: 23',
  '',
  'GET / HTTP/1.1',
  'Dummy: GET /admin HTTP/1.1',
  'Host: localhost',
  '',
  '',
].join('\r\n');

const server = http.createServer(common.mustNotCall());

server.listen(0, common.mustSucceed(() => {
  const client = net.connect(server.address().port, 'localhost');

  let response = '';

  }));
  client.write(msg);
  client.resume();
}));