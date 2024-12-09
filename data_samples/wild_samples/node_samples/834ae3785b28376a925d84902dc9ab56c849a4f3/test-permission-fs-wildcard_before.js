  if (!common.isWindows) {
    const { status, stderr } = spawnSync(
      process.execPath,
      [
        '--experimental-permission',
        '--allow-fs-read=/a/b/*',
        '--allow-fs-read=/a/b/d',
        '-e',
        `
        const assert = require('assert')
        assert.ok(process.permission.has('fs.read', '/a/b/c'));
        assert.ok(!process.permission.has('fs.read', '/a/c/c'));
      `,
      ]
    );
    assert.strictEqual(status, 0, stderr.toString());
  }
}