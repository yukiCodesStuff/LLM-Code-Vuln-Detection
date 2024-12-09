    '',
  ].join('\r\n');

  const server = http.createServer(common.mustNotCall());

  server.listen(0, common.mustSucceed(() => {
    const client = net.connect(server.address().port, 'localhost');
