    '',
  ].join('\r\n');

  const server = http.createServer(common.mustCall((request, response) => {
    assert.notStrictEqual(request.url, '/admin');
    response.end('hello world');
  }), 1);

  server.listen(0, common.mustSucceed(() => {
    const client = net.connect(server.address().port, 'localhost');
