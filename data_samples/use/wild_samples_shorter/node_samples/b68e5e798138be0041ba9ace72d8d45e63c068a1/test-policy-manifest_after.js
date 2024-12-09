  assert.match(stderr, /ERR_MANIFEST_DEPENDENCY_MISSING/);
  assert.match(stderr, /does not list os as a dependency specifier for conditions: require, node, node-addons/);
}

{
  const policyFilepath = fixtures.path('policy-manifest', 'onerror-exit.json');
  const mainModuleBypass = fixtures.path('policy-manifest', 'module-constructor-bypass.js');
  const result = spawnSync(process.execPath, [
    '--experimental-policy',
    policyFilepath,
    mainModuleBypass,
  ]);
  assert.notStrictEqual(result.status, 0);
  const stderr = result.stderr.toString();
  assert.match(stderr, /TypeError/);
}

{
  const policyFilepath = fixtures.path('policy-manifest', 'manifest-impersonate.json');
  const createRequireBypass = fixtures.path('policy-manifest', 'createRequire-bypass.js');
  const result = spawnSync(process.execPath, [
    '--experimental-policy',
    policyFilepath,
    createRequireBypass,
  ]);

  assert.notStrictEqual(result.status, 0);
  const stderr = result.stderr.toString();
  assert.match(stderr, /TypeError/);
}

{
  const policyFilepath = fixtures.path('policy-manifest', 'onerror-exit.json');
  const mainModuleBypass = fixtures.path('policy-manifest', 'main-constructor-bypass.js');
  const result = spawnSync(process.execPath, [
    '--experimental-policy',
    policyFilepath,
    mainModuleBypass,
  ]);

  assert.notStrictEqual(result.status, 0);
  const stderr = result.stderr.toString();
  assert.match(stderr, /TypeError/);
}

{
  const policyFilepath = fixtures.path('policy-manifest', 'onerror-exit.json');
  const mainModuleBypass = fixtures.path('policy-manifest', 'main-constructor-extensions-bypass.js');
  const result = spawnSync(process.execPath, [
    '--experimental-policy',
    policyFilepath,
    mainModuleBypass,
  ]);

  assert.notStrictEqual(result.status, 0);
  const stderr = result.stderr.toString();
  assert.match(stderr, /TypeError/);
}