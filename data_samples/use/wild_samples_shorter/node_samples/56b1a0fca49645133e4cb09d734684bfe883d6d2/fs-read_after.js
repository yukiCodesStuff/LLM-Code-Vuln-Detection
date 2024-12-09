  });
}

// fs.watchFile
{
  assert.throws(() => {
    fs.watchFile(blockedFile, common.mustNotCall());
  }, common.expectsError({
    code: 'ERR_ACCESS_DENIED',
    permission: 'FileSystemRead',
    resource: path.toNamespacedPath(blockedFile),
  }));
}

// fs.rename
{
  assert.throws(() => {
    fs.rename(blockedFile, 'newfile', () => {});