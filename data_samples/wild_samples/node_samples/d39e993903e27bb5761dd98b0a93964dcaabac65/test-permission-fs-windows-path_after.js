{
  const { stdout, status, stderr } = spawnSync(process.execPath, [
    '--experimental-permission', '--allow-fs-write', 'C:\\', '-e',
    `const path = require('path');
     console.log(process.permission.has('fs.write', path.toNamespacedPath('C:\\\\')))`,
  ]);
  assert.strictEqual(stdout.toString(), 'true\n', stderr.toString());
  assert.strictEqual(status, 0);
}

{
  const { stdout, status, stderr } = spawnSync(process.execPath, [
    '--experimental-permission', '--allow-fs-write', 'C:\\*', '-e',
    "console.log(process.permission.has('fs.write', '\\\\\\\\A\\\\C:\\Users'))",
  ]);
  assert.strictEqual(stdout.toString(), 'false\n', stderr.toString());
  assert.strictEqual(status, 0);
}