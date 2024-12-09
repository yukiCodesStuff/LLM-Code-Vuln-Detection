  if (typeof options === 'function') {
    callback = options;
    options = kEmptyObject;
  }
  callback = makeStatsCallback(callback);
  path = getValidatedPath(path);

  const req = new FSReqCallback(options.bigint);
  req.oncomplete = callback;
  binding.lstat(path, options.bigint, req);
}

/**
 * Asynchronously gets the stats of a file.
 * @param {string | Buffer | URL} path
 * @param {{ bigint?: boolean; }} [options]
 * @param {(
 *   err?: Error,
 *   stats?: Stats
 *   ) => any} callback
 * @returns {void}
 */
function stat(path, options = { bigint: false }, callback) {
  if (typeof options === 'function') {
    callback = options;
    options = kEmptyObject;
  }
  callback = makeStatsCallback(callback);

  const req = new FSReqCallback(options.bigint);
  req.oncomplete = callback;
  binding.stat(getValidatedPath(path), options.bigint, req);
}

function statfs(path, options = { bigint: false }, callback) {
  if (typeof options === 'function') {
    callback = options;
    options = kEmptyObject;
  }
  validateFunction(callback, 'cb');
  path = getValidatedPath(path);
  const req = new FSReqCallback(options.bigint);
  req.oncomplete = (err, stats) => {
    if (err) {
      return callback(err);
    }
 * @param {{
 *   bigint?: boolean;
 *   throwIfNoEntry?: boolean;
 *   }} [options]
 * @returns {Stats | undefined}
 */
function lstatSync(path, options = { bigint: false, throwIfNoEntry: true }) {
  const stats = binding.lstat(
    getValidatedPath(path),
    options.bigint,
    undefined,
    options.throwIfNoEntry,
  );

  if (stats === undefined) {
    return;
  }
  return getStatsFromBinding(stats);
}