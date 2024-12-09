  assert.ok(!Buffer.isEncoding(encoding));
  assert.throws(() => Buffer.alloc(9).write('foo', encoding), error);
}