{
  const msg = [
    'POST / HTTP/1.1',
    'Host: 127.0.0.1',
    'Transfer-Encoding: chunked',
    ' , chunked-false',
    'Connection: upgrade',
    '',
    '1',
    'A',
    '0',
    '',
    'GET /flag HTTP/1.1',
    'Host: 127.0.0.1',
    '',
    '',
  ].join('\r\n');

  const server = http.createServer(common.mustCall((request, response) => {
    assert.notStrictEqual(request.url, '/admin');
    response.end('hello world');
  }), 1);

  server.listen(0, common.mustSucceed(() => {
    const client = net.connect(server.address().port, 'localhost');

    client.on('end', common.mustCall(function() {
      server.close();
    }));

    client.write(msg);
    client.resume();
  }));
}