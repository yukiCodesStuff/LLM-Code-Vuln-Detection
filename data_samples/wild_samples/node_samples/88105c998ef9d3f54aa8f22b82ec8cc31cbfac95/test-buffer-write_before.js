  const error = common.expectsError({
    code: 'ERR_UNKNOWN_ENCODING',
    type: TypeError,
    message: `Unknown encoding: ${encoding}`
  });

  assert.ok(!Buffer.isEncoding(encoding));
  assert.throws(() => Buffer.alloc(9).write('foo', encoding), error);
}