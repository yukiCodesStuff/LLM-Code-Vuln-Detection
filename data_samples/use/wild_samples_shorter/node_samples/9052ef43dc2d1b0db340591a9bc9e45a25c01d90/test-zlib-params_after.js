const chunk1 = file.slice(0, chunkSize);
const chunk2 = file.slice(chunkSize);
const blkhdr = Buffer.from([0x00, 0x5a, 0x82, 0xa5, 0x7d]);
const blkftr = Buffer.from('010000ffff7dac3072', 'hex');
const expected = Buffer.concat([blkhdr, chunk2, blkftr]);
const bufs = [];

function read() {
  let buf;
  while ((buf = deflater.read()) !== null) {
    bufs.push(buf);
  }
}

deflater.write(chunk1, function() {
  deflater.params(0, zlib.constants.Z_DEFAULT_STRATEGY, function() {
    while (deflater.read());

    deflater.on('readable', read);

    deflater.end(chunk2);
  });
  while (deflater.read());
});

process.once('exit', function() {
  const actual = Buffer.concat(bufs);
  assert.deepStrictEqual(actual, expected);
});