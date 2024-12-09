function possiblyTransformPath(path) {
  if (permission.isEnabled()) {
    if (typeof path === 'string') {
      return pathModule.resolve(path);
    }
  }
  return path;
}