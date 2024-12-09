function fchmod(fd, mode, callback) {
  mode = parseFileMode(mode, 'mode');
  callback = makeCallback(callback);

  if (permission.isEnabled()) {
    callback(new ERR_ACCESS_DENIED('fchmod API is disabled when Permission Model is enabled.'));
    return;
  }
  if (permission.isEnabled()) {
    callback(new ERR_ACCESS_DENIED('fchmod API is disabled when Permission Model is enabled.'));
    return;
  }

  const req = new FSReqCallback();
  req.oncomplete = callback;
  binding.fchmod(fd, mode, req);
}

/**
 * Synchronously sets the permissions on the file.
 * @param {number} fd
 * @param {string | number} mode
 * @returns {void}
 */
function fchmodSync(fd, mode) {
  if (permission.isEnabled()) {
    throw new ERR_ACCESS_DENIED('fchmod API is disabled when Permission Model is enabled.');
  }
  binding.fchmod(
    fd,
    parseFileMode(mode, 'mode'),
  );
}
function fchown(fd, uid, gid, callback) {
  validateInteger(uid, 'uid', -1, kMaxUserId);
  validateInteger(gid, 'gid', -1, kMaxUserId);
  callback = makeCallback(callback);
  if (permission.isEnabled()) {
    callback(new ERR_ACCESS_DENIED('fchown API is disabled when Permission Model is enabled.'));
    return;
  }
function fchownSync(fd, uid, gid) {
  validateInteger(uid, 'uid', -1, kMaxUserId);
  validateInteger(gid, 'gid', -1, kMaxUserId);
  if (permission.isEnabled()) {
    throw new ERR_ACCESS_DENIED('fchown API is disabled when Permission Model is enabled.');
  }