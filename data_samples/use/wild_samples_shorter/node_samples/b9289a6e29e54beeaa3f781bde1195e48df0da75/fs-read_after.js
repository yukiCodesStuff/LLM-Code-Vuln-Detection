  }, common.expectsError({
    code: 'ERR_ACCESS_DENIED',
    permission: 'FileSystemRead',
    // cpSync calls lstatSync before reading blockedFile
    resource: blockedFile,
  }));
  assert.throws(() => {
    fs.cpSync(blockedFileURL, path.join(blockedFolder, 'any-other-file'));
  }, common.expectsError({
    code: 'ERR_ACCESS_DENIED',
    permission: 'FileSystemRead',
    // cpSync calls lstatSync before reading blockedFile
    resource: blockedFile,
  }));
  assert.throws(() => {
    fs.cpSync(blockedFile, path.join(__dirname, 'any-other-file'));
  }, common.expectsError({
    code: 'ERR_ACCESS_DENIED',
    permission: 'FileSystemRead',
    resource: blockedFile,
  }));
}

// fs.open
    permission: 'FileSystemRead',
    resource: blockedFolder,
  }));
}

// fs.lstat
{
  assert.throws(() => {
    fs.lstatSync(blockedFile);
  }, common.expectsError({
    code: 'ERR_ACCESS_DENIED',
  }));
  assert.throws(() => {
    fs.lstatSync(path.join(blockedFolder, 'anyfile'));
  }, common.expectsError({
    code: 'ERR_ACCESS_DENIED',
  }));

  // doesNotThrow
  fs.lstat(regularFile, (err) => {
    assert.ifError(err);
  });
}