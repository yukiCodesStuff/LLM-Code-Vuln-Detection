  }, {
    code: 'ERR_ACCESS_DENIED',
    permission: 'FileSystemWrite',
  });
}

// fs.unlink
{
  assert.throws(() => {
    fs.unlink(blockedFile, (err) => {
      assert.ifError(err);
    });
  }, {
    code: 'ERR_ACCESS_DENIED',
    permission: 'FileSystemWrite',
    resource: path.toNamespacedPath(blockedFile),
  });
}