Buffer.alloc = function alloc(size, fill, encoding) {
  assertSize(size);
  if (fill !== undefined && fill !== 0 && size > 0) {
    return _fill(createUnsafeBuffer(size), fill, encoding);
  }
  return new FastBuffer(size);
};
