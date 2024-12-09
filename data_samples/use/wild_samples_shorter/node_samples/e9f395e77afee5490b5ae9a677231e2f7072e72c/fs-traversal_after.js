const fs = require('fs');
const path = require('path');

const { resolve } = path;
// This should not affect how the permission model resolves paths.
try {
  path.resolve = (s) => s;
  assert.fail('should not be called');
} catch {}

const blockedFolder = process.env.BLOCKEDFOLDER;
const allowedFolder = process.env.ALLOWEDFOLDER;
const traversalPath = allowedFolder + '/../file.md';
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

  assert.throws(() => {
    fs.readFile(cwd, common.mustNotCall());
  }, common.expectsError({
    code: 'ERR_ACCESS_DENIED',
    permission: 'FileSystemRead',
    resource: resolve(cwd.toString()),
  }));
}

// Monkey-patching Buffer internals should also not allow path traversal.
{
  const extraChars = '.'.repeat(40);
  const traversalPathWithExtraChars = traversalPath + extraChars;