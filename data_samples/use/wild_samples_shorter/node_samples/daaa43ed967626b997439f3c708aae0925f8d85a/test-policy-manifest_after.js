
  assert.strictEqual(result.status, 0);
}

{
  const policyFilepath = fixtures.path('policy-manifest', 'onerror-exit.json');
  const mainModuleBypass = fixtures.path('policy-manifest', 'main-module-proto-bypass.js');
  const result = spawnSync(process.execPath, [
    '--experimental-policy',
    policyFilepath,
    mainModuleBypass,
  ]);

  assert.notStrictEqual(result.status, 0);
  const stderr = result.stderr.toString();
  assert.match(stderr, /ERR_MANIFEST_DEPENDENCY_MISSING/);
  assert.match(stderr, /does not list os as a dependency specifier for conditions: require, node, node-addons/);
}