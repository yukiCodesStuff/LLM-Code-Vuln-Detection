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
  assert.ok(!process.permission.has('fs.write', traversalFolderPath));
}