  }
  callback = makeStatsCallback(callback);
  path = getValidatedPath(path);

  const req = new FSReqCallback(options.bigint);
  req.oncomplete = callback;
  binding.lstat(path, options.bigint, req);
 * @returns {Stats | undefined}
 */
function lstatSync(path, options = { bigint: false, throwIfNoEntry: true }) {
  const stats = binding.lstat(
    getValidatedPath(path),
    options.bigint,
    undefined,