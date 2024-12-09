}

{
  const { status, stdout } = spawnSync(
    process.execPath,
    [
      '--experimental-permission',
      '--allow-fs-write', '/tmp/', '-e',
      `console.log(process.permission.has("fs"));
      console.log(process.permission.has("fs.read"));
      console.log(process.permission.has("fs.write"));
      console.log(process.permission.has("fs.write", "/tmp/"));`,