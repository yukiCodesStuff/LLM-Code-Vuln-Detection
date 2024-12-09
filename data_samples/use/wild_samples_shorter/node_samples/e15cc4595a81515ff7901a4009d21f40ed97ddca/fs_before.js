  // The underlying implementation here returns the Blob synchronously for now.
  // To give ourselves flexibility to maybe return the Blob asynchronously,
  // this API returns a Promise.
  return PromiseResolve(createBlobFromFilePath(getValidatedPath(path), { type }));
}

/**
 * Reads file from the specified `fd` (file descriptor).