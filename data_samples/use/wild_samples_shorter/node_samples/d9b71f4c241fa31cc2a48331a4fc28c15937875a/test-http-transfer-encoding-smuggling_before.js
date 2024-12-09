'use strict';

const common = require('../common');

const http = require('http');
const net = require('net');

  '',
].join('\r\n');

// Verify that the server is called only once even with a smuggled request.

const server = http.createServer(common.mustCall((req, res) => {
  res.end();
}, 1));

function send(next) {
  const client = net.connect(server.address().port, 'localhost');
  client.setEncoding('utf8');
  client.on('error', common.mustNotCall());
  client.on('end', next);
  client.write(msg);
  client.resume();
}

server.listen(0, common.mustSucceed(() => {
  send(common.mustCall(() => {
    server.close();
  }));
}));