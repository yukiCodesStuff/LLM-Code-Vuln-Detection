process.on('exit', function() {
  hooks.disable();

  verifyGraph(
    hooks,
    [ { type: 'TCPSERVERWRAP',
        id: 'tcpserver:1',
        triggerAsyncId: null },
      { type: 'TCPWRAP', id: 'tcp:1', triggerAsyncId: 'tcpserver:1' },
      { type: 'TCPCONNECTWRAP',
        id: 'tcpconnect:1',
        triggerAsyncId: 'tcp:1' },
      { type: 'HTTPPARSER',
        id: 'httpparser:1',
        triggerAsyncId: 'tcpserver:1' },
      { type: 'TCPWRAP', id: 'tcp:2', triggerAsyncId: 'tcpserver:1' },
      { type: 'Timeout', id: 'timeout:1', triggerAsyncId: 'tcp:2' },
      { type: 'HTTPPARSER',
        id: 'httpparser:2',
        triggerAsyncId: 'tcp:2' },
      { type: 'Timeout',
        id: 'timeout:2',
        triggerAsyncId: 'httpparser:2' },
      { type: 'SHUTDOWNWRAP',
        id: 'shutdown:1',
        triggerAsyncId: 'tcp:2' } ]
  );
});