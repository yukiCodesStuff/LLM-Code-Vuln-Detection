    servername: 'localhost'
  }, common.mustNotCall());
  socket.on('data', common.mustNotCall());
  socket.on('error', common.mustCall(function(err) {
    authorized();
  }));
  socket.end('ng');