'use strict'

const common = require('../../common');

const assert = require('assert');
const fs = require('fs');
const path = require('path');

const { resolve } = path;
// This should not affect how the permission model resolves paths.
try {
  path.resolve = (s) => s;
  assert.fail('should not be called');
} catch {}
  }, common.expectsError({
    code: 'ERR_ACCESS_DENIED',
    permission: 'FileSystemRead',
    resource: resolve(traversalPath),
  }));
}

// Monkey-patching path module should also not allow path traversal.
{
  const fs = require('fs');
  const path = require('path');

  const cwd = Buffer.from('.');
  try {
    path.toNamespacedPath = (path) => { return traversalPath; };
    assert.fail('should throw error when pacthing');
  } catch { }