  code: 'ERR_INVALID_ARG_VALUE',
  type: TypeError
});

common.expectsError(() => {
  Buffer.alloc(40, 'x', 20);
}, {
  code: 'ERR_INVALID_ARG_TYPE',
  type: TypeError
});