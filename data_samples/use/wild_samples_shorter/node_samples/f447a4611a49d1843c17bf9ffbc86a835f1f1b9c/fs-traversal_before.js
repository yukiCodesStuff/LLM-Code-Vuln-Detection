const traversalPath = allowedFolder + '../file.md';
const traversalFolderPath = allowedFolder + '../folder';
const bufferTraversalPath = Buffer.from(allowedFolder + '../file.md');

{
  assert.ok(process.permission.has('fs.read', allowedFolder));
  assert.ok(process.permission.has('fs.write', allowedFolder));
  }));
}

{
  assert.ok(!process.permission.has('fs.read', traversalPath));
  assert.ok(!process.permission.has('fs.write', traversalPath));
  assert.ok(!process.permission.has('fs.read', traversalFolderPath));