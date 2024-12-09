  const error = common.expectsError({
    code: 'ERR_UNKNOWN_ENCODING',
    type: TypeError,
    message: `Unknown encoding: ${encoding}`
  });

  assert.ok(!Buffer.isEncoding(encoding));
  assert.throws(() => Buffer.alloc(9).write('foo', encoding), error);
}

// UCS-2 overflow CVE-2018-12115
for (let i = 1; i < 4; i++) {
  // Allocate two Buffers sequentially off the pool. Run more than once in case
  // we hit the end of the pool and don't get sequential allocations
  const x = Buffer.allocUnsafe(4).fill(0);
  const y = Buffer.allocUnsafe(4).fill(1);
  // Should not write anything, pos 3 doesn't have enough room for a 16-bit char
  assert.strictEqual(x.write('ыыыыыы', 3, 'ucs2'), 0);
  // CVE-2018-12115 experienced via buffer overrun to next block in the pool
  assert.strictEqual(Buffer.compare(y, Buffer.alloc(4, 1)), 0);
}

// Should not write any data when there is no space for 16-bit chars
const z = Buffer.alloc(4, 0);
assert.strictEqual(z.write('\u0001', 3, 'ucs2'), 0);
assert.strictEqual(Buffer.compare(z, Buffer.alloc(4, 0)), 0);

// Large overrun could corrupt the process
assert.strictEqual(Buffer.alloc(4)
  .write('ыыыыыы'.repeat(100), 3, 'utf16le'), 0);