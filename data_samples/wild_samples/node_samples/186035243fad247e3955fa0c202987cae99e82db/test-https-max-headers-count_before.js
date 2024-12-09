const serverOptions = {
  key: fixtures.readKey('agent1-key.pem'),
  cert: fixtures.readKey('agent1-cert.pem')
};

let requests = 0;
let responses = 0;

const headers = {};
const N = 2000;
for (let i = 0; i < N; ++i) {
  headers[`key${i}`] = i;
}
server.listen(0, common.mustCall(() => {
  const maxAndExpected = [ // for client
    [20, 20],
    [1200, 1200],
    [0, N + 3] // Connection, Date and Transfer-Encoding
  ];
  const doRequest = common.mustCall(() => {
    const max = maxAndExpected[responses][0];
    const expected = maxAndExpected[responses][1];
    const req = https.request({
      port: server.address().port,
      headers: headers,
      rejectUnauthorized: false
    }, (res) => {
      assert.strictEqual(Object.keys(res.headers).length, expected);
      res.on('end', () => {
        if (++responses < maxAndExpected.length) {
          doRequest();
        } else {
          server.close();
        }
      });
      res.resume();
    });
    req.maxHeadersCount = max;
    req.end();
  }, 3);
  doRequest();
}));