const traversalPath = allowedFolder + '../file.md';
const traversalFolderPath = allowedFolder + '../folder';
const bufferTraversalPath = Buffer.from(allowedFolder + '../file.md');
const uint8ArrayTraversalPath = new TextEncoder().encode(traversalPath);

{
  assert.ok(process.permission.has('fs.read', allowedFolder));
  assert.ok(process.permission.has('fs.write', allowedFolder));
  }));
}

{
  assert.throws(() => {
    fs.readFile(uint8ArrayTraversalPath, (error) => {
      assert.ifError(error);
    });
  }, common.expectsError({
    code: 'ERR_ACCESS_DENIED',
    permission: 'FileSystemRead',
    resource: resolve(traversalPath),
  }));
}

{
  assert.ok(!process.permission.has('fs.read', traversalPath));
  assert.ok(!process.permission.has('fs.write', traversalPath));
  assert.ok(!process.permission.has('fs.read', traversalFolderPath));