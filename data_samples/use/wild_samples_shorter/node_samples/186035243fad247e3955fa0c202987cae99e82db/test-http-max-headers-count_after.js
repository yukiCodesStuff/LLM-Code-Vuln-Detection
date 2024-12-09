let responses = 0;

const headers = {};
const N = 100;
for (let i = 0; i < N; ++i) {
  headers[`key${i}`] = i;
}

const maxAndExpected = [ // for server
  [50, 50],
  [1500, 102],
  [0, N + 2] // Host and Connection
];
let max = maxAndExpected[requests][0];
let expected = maxAndExpected[requests][1];
server.listen(0, function() {
  const maxAndExpected = [ // for client
    [20, 20],
    [1200, 103],
    [0, N + 3] // Connection, Date and Transfer-Encoding
  ];
  doRequest();
