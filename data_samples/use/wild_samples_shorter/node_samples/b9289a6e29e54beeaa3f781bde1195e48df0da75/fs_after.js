  }
  callback = makeStatsCallback(callback);
  path = getValidatedPath(path);
  if (permission.isEnabled() && !permission.has('fs.read', path)) {
    callback(new ERR_ACCESS_DENIED('Access to this API has been restricted', 'FileSystemRead', path));
    return;
  }

  const req = new FSReqCallback(options.bigint);
  req.oncomplete = callback;
  binding.lstat(path, options.bigint, req);
 * @returns {Stats | undefined}
 */
function lstatSync(path, options = { bigint: false, throwIfNoEntry: true }) {
  path = getValidatedPath(path);
  if (permission.isEnabled() && !permission.has('fs.read', path)) {
    throw new ERR_ACCESS_DENIED('Access to this API has been restricted', 'FileSystemRead', path);
  }
  const stats = binding.lstat(
    getValidatedPath(path),
    options.bigint,
    undefined,