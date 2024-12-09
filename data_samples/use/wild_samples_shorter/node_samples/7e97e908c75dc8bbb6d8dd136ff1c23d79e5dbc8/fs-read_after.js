    resource: path.toNamespacedPath(blockedFile),
  }));
}

// fs.exists
{
  // It will return false (without performing IO) when permissions is not met
  fs.exists(blockedFile, (exists) => {
    assert.equal(exists, false);
  });
}

// fs.statfs
{
  assert.throws(() => {
    fs.statfs(blockedFile, () => {});
  }, common.expectsError({
    code: 'ERR_ACCESS_DENIED',
    permission: 'FileSystemRead',
    resource: path.toNamespacedPath(blockedFile),
  }));
}