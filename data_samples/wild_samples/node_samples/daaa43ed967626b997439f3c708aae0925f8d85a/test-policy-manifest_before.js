{
  const policyFilepath = fixtures.path('policy-manifest', 'onerror-resource-exit.json');
  const objectDefinePropertyBypass = fixtures.path('policy-manifest', 'object-define-property-bypass.js');
  const result = spawnSync(process.execPath, [
    '--experimental-policy',
    policyFilepath,
    objectDefinePropertyBypass,
  ]);

  assert.strictEqual(result.status, 0);
}