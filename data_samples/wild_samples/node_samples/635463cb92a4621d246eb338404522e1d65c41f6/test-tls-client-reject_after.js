  const socket = tls.connect(server.address().port, {
    servername: 'localhost'
  }, common.mustNotCall());
  socket.on('data', common.mustNotCall());
  socket.on('error', common.mustCall(function(err) {
    rejectUnauthorizedUndefined();
  }));
  socket.end('ng');
}