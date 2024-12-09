const fs = require('fs');
const path = require('path');

const blockedFolder = process.env.BLOCKEDFOLDER;
const allowedFolder = process.env.ALLOWEDFOLDER;
const traversalPath = allowedFolder + '../file.md';
const traversalFolderPath = allowedFolder + '../folder';
  }, common.expectsError({
    code: 'ERR_ACCESS_DENIED',
    permission: 'FileSystemWrite',
    resource: path.toNamespacedPath(path.resolve(traversalPath)),
  }));
}

{
  }, common.expectsError({
    code: 'ERR_ACCESS_DENIED',
    permission: 'FileSystemRead',
    resource: path.toNamespacedPath(path.resolve(traversalPath)),
  }));
}

{
  }, common.expectsError({
    code: 'ERR_ACCESS_DENIED',
    permission: 'FileSystemWrite',
    resource: path.resolve(traversalFolderPath + 'XXXXXX'),
  }));
}

{
  }, common.expectsError({
    code: 'ERR_ACCESS_DENIED',
    permission: 'FileSystemWrite',
    resource: path.resolve(traversalFolderPath + 'XXXXXX'),
  }));
}

{
  }, common.expectsError({
    code: 'ERR_ACCESS_DENIED',
    permission: 'FileSystemRead',
    resource: path.resolve(traversalPath),
  }));
}

{