  }));
}

// Monkey-patching Buffer internals should also not allow path traversal.
{
  const extraChars = '.'.repeat(40);
  const traversalPathWithExtraChars = traversalPath + extraChars;
  const traversalPathWithExtraBytes = Buffer.from(traversalPathWithExtraChars);

  Buffer.prototype.utf8Write = ((w) => function(str, ...args) {
    assert.strictEqual(str, resolve(traversalPath) + extraChars);
    return w.apply(this, [traversalPath, ...args]);
  })(Buffer.prototype.utf8Write);

  // Sanity check (remove if the internals of Buffer.from change):
  // The custom implementation of utf8Write should cause Buffer.from() to encode
  // traversalPath instead of the sanitized output of resolve().
  assert.strictEqual(Buffer.from(resolve(traversalPathWithExtraChars)).toString(), traversalPath);

  assert.throws(() => {
    fs.readFile(traversalPathWithExtraBytes, common.mustNotCall());
  }, common.expectsError({
    code: 'ERR_ACCESS_DENIED',
    permission: 'FileSystemRead',
    resource: resolve(traversalPathWithExtraChars),
  }));

  assert.throws(() => {
    fs.readFile(new TextEncoder().encode(traversalPathWithExtraBytes.toString()), common.mustNotCall());
  }, common.expectsError({
    code: 'ERR_ACCESS_DENIED',
    permission: 'FileSystemRead',
    resource: resolve(traversalPathWithExtraChars),
  }));
}

{
  assert.ok(!process.permission.has('fs.read', traversalPath));
  assert.ok(!process.permission.has('fs.write', traversalPath));
  assert.ok(!process.permission.has('fs.read', traversalFolderPath));