'use strict';

const common = require('../common');
const assert = require('assert');

const http = require('http');
const net = require('net');

  '',
].join('\r\n');

const server = http.createServer(common.mustNotCall((req, res) => {
  res.end();
}, 1));

server.listen(0, common.mustSucceed(() => {
  const client = net.connect(server.address().port, 'localhost');

  let response = '';

  // Verify that the server listener is never called

  client.on('data', common.mustCall((chunk) => {
    response += chunk.toString('utf-8');
  }));

  client.setEncoding('utf8');
  client.on('error', common.mustNotCall());
  client.on('end', common.mustCall(() => {
    assert.strictEqual(
      response,
      'HTTP/1.1 400 Bad Request\r\nConnection: close\r\n\r\n'
    );
    server.close();
  }));
  client.write(msg);
  client.resume();
}));