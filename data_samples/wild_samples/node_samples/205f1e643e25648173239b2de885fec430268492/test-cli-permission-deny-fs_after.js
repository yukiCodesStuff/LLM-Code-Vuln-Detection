{
  const { status, stdout } = spawnSync(
    process.execPath,
    [
      '--experimental-permission', '-e',
      `console.log(process.permission.has("fs"));
       console.log(process.permission.has("fs.read"));
       console.log(process.permission.has("fs.write"));`,
    ]
  );

  const [fs, fsIn, fsOut] = stdout.toString().split('\n');
  assert.strictEqual(fs, 'false');
  assert.strictEqual(fsIn, 'false');
  assert.strictEqual(fsOut, 'false');
  assert.strictEqual(status, 0);
}

{
  const tmpPath = path.resolve('/tmp/');
  const { status, stdout } = spawnSync(
    process.execPath,
    [
      '--experimental-permission',
      '--allow-fs-write', tmpPath, '-e',
      `console.log(process.permission.has("fs"));
      console.log(process.permission.has("fs.read"));
      console.log(process.permission.has("fs.write"));
      console.log(process.permission.has("fs.write", "/tmp/"));`,
    ]
  );
  const [fs, fsIn, fsOut, fsOutAllowed] = stdout.toString().split('\n');
  assert.strictEqual(fs, 'false');
  assert.strictEqual(fsIn, 'false');
  assert.strictEqual(fsOut, 'false');
  assert.strictEqual(fsOutAllowed, 'true');
  assert.strictEqual(status, 0);
}