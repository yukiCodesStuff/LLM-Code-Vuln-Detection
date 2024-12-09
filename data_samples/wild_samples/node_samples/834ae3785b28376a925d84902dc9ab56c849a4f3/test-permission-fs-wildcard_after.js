  if (!common.isWindows) {
    const { status, stderr } = spawnSync(
      process.execPath,
      [
        '--experimental-permission',
        '--allow-fs-read=/a/b/*',
        '--allow-fs-read=/a/b/d',
        '--allow-fs-read=/etc/passwd.*',
        '--allow-fs-read=/home/*.js',
        '-e',
        `
        const assert = require('assert')
        assert.ok(process.permission.has('fs.read', '/a/b/c'));
        assert.ok(!process.permission.has('fs.read', '/a/c/c'));
        assert.ok(!process.permission.has('fs.read', '/etc/passwd'));
        assert.ok(process.permission.has('fs.read', '/home/another-file.md'));
      `,
      ]
    );
    assert.strictEqual(status, 0, stderr.toString());
  }
}