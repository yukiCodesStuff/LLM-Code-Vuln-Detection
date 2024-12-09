  }, common.expectsError({
    code: 'ERR_ACCESS_DENIED',
    permission: 'FileSystemRead',
    // cpSync calls statSync before reading blockedFile
    resource: path.toNamespacedPath(blockedFolder),
  }));
  assert.throws(() => {
    fs.cpSync(blockedFileURL, path.join(blockedFolder, 'any-other-file'));
  }, common.expectsError({
    code: 'ERR_ACCESS_DENIED',
    permission: 'FileSystemRead',
    // cpSync calls statSync before reading blockedFile
    resource: path.toNamespacedPath(blockedFolder),
  }));
  assert.throws(() => {
    fs.cpSync(blockedFile, path.join(__dirname, 'any-other-file'));
  }, common.expectsError({
    code: 'ERR_ACCESS_DENIED',
    permission: 'FileSystemRead',
    resource: path.toNamespacedPath(__dirname),
  }));
}

// fs.open
{
  fs.open(blockedFile, 'r', common.expectsError({
    code: 'ERR_ACCESS_DENIED',
    permission: 'FileSystemRead',
    resource: path.toNamespacedPath(blockedFile),
  }));
  assert.throws(() => {
    fs.openSync(blockedFileURL, 'r');
  }, common.expectsError({
    code: 'ERR_ACCESS_DENIED',
    permission: 'FileSystemRead',
    resource: path.toNamespacedPath(blockedFile),
  }));
  assert.throws(() => {
    fs.openSync(path.join(blockedFolder, 'anyfile'), 'r');
  }, common.expectsError({
    code: 'ERR_ACCESS_DENIED',
    permission: 'FileSystemRead',
    resource: path.toNamespacedPath(path.join(blockedFolder, 'anyfile')),
  }));

  // doesNotThrow
  fs.open(regularFile, 'r', (err) => {
    assert.ifError(err);
  });

  // Extra flags should not enable trivially bypassing all restrictions.
  // See https://github.com/nodejs/node/issues/47090.
  assert.throws(() => {
    fs.openSync(blockedFile, fs.constants.O_RDONLY | fs.constants.O_NOCTTY);
  }, {
    code: 'ERR_ACCESS_DENIED',
    permission: 'FileSystemRead',
  });
  fs.open(blockedFile, fs.constants.O_RDWR | 0x10000000, common.expectsError({
    code: 'ERR_ACCESS_DENIED',
    permission: 'FileSystemRead',
  }));
}

// fs.opendir
{
  fs.opendir(blockedFolder, common.expectsError({
    code: 'ERR_ACCESS_DENIED',
    permission: 'FileSystemRead',
    resource: path.toNamespacedPath(blockedFolder),
  }));
  assert.throws(() => {
    fs.opendirSync(blockedFolder);
  }, common.expectsError({
    code: 'ERR_ACCESS_DENIED',
    permission: 'FileSystemRead',
    resource: path.toNamespacedPath(blockedFolder),
  }));
  // doesNotThrow
  fs.opendir(allowedFolder, (err, dir) => {
    assert.ifError(err);
    dir.closeSync();
  });
}

// fs.readdir
{
  fs.readdir(blockedFolder, common.expectsError({
    code: 'ERR_ACCESS_DENIED',
    permission: 'FileSystemRead',
    resource: path.toNamespacedPath(blockedFolder),
  }));
  assert.throws(() => {
    fs.readdirSync(blockedFolder);
  }, common.expectsError({
    code: 'ERR_ACCESS_DENIED',
    permission: 'FileSystemRead',
    resource: path.toNamespacedPath(blockedFolder),
  }));

  // doesNotThrow
  fs.readdir(allowedFolder, (err) => {
    assert.ifError(err);
  });
}

// fs.watch
{
  assert.throws(() => {
    fs.watch(blockedFile, () => {});
  }, common.expectsError({
    code: 'ERR_ACCESS_DENIED',
    permission: 'FileSystemRead',
    resource: path.toNamespacedPath(blockedFile),
  }));
  assert.throws(() => {
    fs.watch(blockedFileURL, () => {});
  }, common.expectsError({
    code: 'ERR_ACCESS_DENIED',
    permission: 'FileSystemRead',
    resource: path.toNamespacedPath(blockedFile),
  }));

  // doesNotThrow
  fs.readdir(allowedFolder, (err) => {
    assert.ifError(err);
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
  assert.throws(() => {
    fs.watchFile(blockedFileURL, common.mustNotCall());
  }, common.expectsError({
    code: 'ERR_ACCESS_DENIED',
    permission: 'FileSystemRead',
    resource: path.toNamespacedPath(blockedFile),
  }));
}

// fs.rename
{
  fs.rename(blockedFile, 'newfile', common.expectsError({
    code: 'ERR_ACCESS_DENIED',
    permission: 'FileSystemRead',
    resource: path.toNamespacedPath(blockedFile),
  }));
  assert.throws(() => {
    fs.renameSync(blockedFile, 'newfile');
  }, common.expectsError({
    code: 'ERR_ACCESS_DENIED',
    permission: 'FileSystemRead',
    resource: path.toNamespacedPath(blockedFile),
  }));
  assert.throws(() => {
    fs.renameSync(blockedFileURL, 'newfile');
  }, common.expectsError({
    code: 'ERR_ACCESS_DENIED',
    permission: 'FileSystemRead',
    resource: path.toNamespacedPath(blockedFile),
  }));
}

// fs.openAsBlob
{
  assert.throws(() => {
    fs.openAsBlob(blockedFile);
  }, common.expectsError({
    code: 'ERR_ACCESS_DENIED',
    permission: 'FileSystemRead',
    resource: path.toNamespacedPath(blockedFile),
  }));
  assert.throws(() => {
    fs.openAsBlob(blockedFileURL);
  }, common.expectsError({
    code: 'ERR_ACCESS_DENIED',
    permission: 'FileSystemRead',
    resource: path.toNamespacedPath(blockedFile),
  }));
}

// fs.exists
{
  // It will return false (without performing IO) when permissions is not met
  fs.exists(blockedFile, (exists) => {
    assert.equal(exists, false);
  });
  fs.exists(blockedFileURL, (exists) => {
    assert.equal(exists, false);
  });
  assert.throws(() => {
    fs.existsSync(blockedFile);
  }, common.expectsError({
    code: 'ERR_ACCESS_DENIED',
    permission: 'FileSystemRead',
    resource: path.toNamespacedPath(blockedFile),
  }));
}

// fs.statfs
{
  fs.statfs(blockedFile, common.expectsError({
    code: 'ERR_ACCESS_DENIED',
    permission: 'FileSystemRead',
    resource: path.toNamespacedPath(blockedFile),
  }));
  assert.throws(() => {
    fs.statfsSync(blockedFile);
  }, common.expectsError({
    code: 'ERR_ACCESS_DENIED',
    permission: 'FileSystemRead',
    resource: path.toNamespacedPath(blockedFile),
  }));
  assert.throws(() => {
    fs.statfsSync(blockedFileURL);
  }, common.expectsError({
    code: 'ERR_ACCESS_DENIED',
    permission: 'FileSystemRead',
    resource: path.toNamespacedPath(blockedFile),
  }));
}

// process.chdir
{
  assert.throws(() => {
    process.chdir(blockedFolder);
  }, common.expectsError({
    code: 'ERR_ACCESS_DENIED',
    permission: 'FileSystemRead',
    resource: blockedFolder,
  }));
}
  }, common.expectsError({
    code: 'ERR_ACCESS_DENIED',
    permission: 'FileSystemRead',
    resource: blockedFolder,
  }));
}