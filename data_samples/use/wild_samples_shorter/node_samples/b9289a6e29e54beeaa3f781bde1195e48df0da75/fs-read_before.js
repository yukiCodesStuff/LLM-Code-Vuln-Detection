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
    permission: 'FileSystemRead',
    resource: blockedFolder,
  }));
}