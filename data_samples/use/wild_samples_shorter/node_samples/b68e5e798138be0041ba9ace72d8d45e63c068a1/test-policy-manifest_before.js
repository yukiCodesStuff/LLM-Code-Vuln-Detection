  assert.match(stderr, /ERR_MANIFEST_DEPENDENCY_MISSING/);
  assert.match(stderr, /does not list os as a dependency specifier for conditions: require, node, node-addons/);
}