const fs = require('fs');
const path = require('path');

// This should not affect how the permission model resolves paths.
const { resolve } = path;
path.resolve = (s) => s;

const blockedFolder = process.env.BLOCKEDFOLDER;
const allowedFolder = process.env.ALLOWEDFOLDER;
const traversalPath = allowedFolder + '../file.md';
const traversalFolderPath = allowedFolder + '../folder';
  }, common.expectsError({
    code: 'ERR_ACCESS_DENIED',
    permission: 'FileSystemWrite',
    resource: path.toNamespacedPath(resolve(traversalPath)),
  }));
}

{
  }, common.expectsError({
    code: 'ERR_ACCESS_DENIED',
    permission: 'FileSystemRead',
    resource: path.toNamespacedPath(resolve(traversalPath)),
  }));
}

{
  }, common.expectsError({
    code: 'ERR_ACCESS_DENIED',
    permission: 'FileSystemWrite',
    resource: resolve(traversalFolderPath + 'XXXXXX'),
  }));
}

{
  }, common.expectsError({
    code: 'ERR_ACCESS_DENIED',
    permission: 'FileSystemWrite',
    resource: resolve(traversalFolderPath + 'XXXXXX'),
  }));
}

{
  }, common.expectsError({
    code: 'ERR_ACCESS_DENIED',
    permission: 'FileSystemRead',
    resource: resolve(traversalPath),
  }));
}

{