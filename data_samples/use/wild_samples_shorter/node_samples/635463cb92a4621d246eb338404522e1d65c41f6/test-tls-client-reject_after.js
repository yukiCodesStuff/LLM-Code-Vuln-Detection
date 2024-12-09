    servername: 'localhost'
  }, common.mustNotCall());
  socket.on('data', common.mustNotCall());
  socket.on('error', common.mustCall(function(err) {
    rejectUnauthorizedUndefined();
  }));
  socket.end('ng');
}

function rejectUnauthorizedUndefined() {
  console.log('reject unauthorized undefined');
  const socket = tls.connect(server.address().port, {
    servername: 'localhost',
    rejectUnauthorized: undefined
  }, common.mustNotCall());
  socket.on('data', common.mustNotCall());
  socket.on('error', common.mustCall(function(err) {
    authorized();
  }));
  socket.end('ng');