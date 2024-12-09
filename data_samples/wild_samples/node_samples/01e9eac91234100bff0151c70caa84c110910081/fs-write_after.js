  }, {
    code: 'ERR_ACCESS_DENIED',
    permission: 'FileSystemWrite',
    resource: path.toNamespacedPath(blockedFile),
  });
}

// fs.fchown with read-only fd
{
  assert.throws(() => {
    // blocked file is allowed to read
    const fd = fs.openSync(blockedFile, 'r');
    fs.fchmod(fd, 777, common.expectsError({
      code: 'ERR_ACCESS_DENIED',
    }));
    fs.fchmodSync(fd, 777);
  }, {
    code: 'ERR_ACCESS_DENIED',
  });
}

// fs.fchmod with read-only fd
{
  assert.throws(() => {
    // blocked file is allowed to read
    const fd = fs.openSync(blockedFile, 'r');
    fs.fchown(fd, 999, 999, common.expectsError({
      code: 'ERR_ACCESS_DENIED',
    }));
    fs.fchownSync(fd, 999, 999);
  }, {
    code: 'ERR_ACCESS_DENIED',
  });
}