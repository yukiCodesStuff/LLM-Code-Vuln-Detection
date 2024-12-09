  // The underlying implementation here returns the Blob synchronously for now.
  // To give ourselves flexibility to maybe return the Blob asynchronously,
  // this API returns a Promise.
  path = getValidatedPath(path);
  return PromiseResolve(createBlobFromFilePath(pathModule.toNamespacedPath(path), { type }));
}

/**
 * Reads file from the specified `fd` (file descriptor).