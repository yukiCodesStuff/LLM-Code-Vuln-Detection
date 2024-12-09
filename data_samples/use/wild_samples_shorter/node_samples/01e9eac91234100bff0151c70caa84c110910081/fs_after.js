  mode = parseFileMode(mode, 'mode');
  callback = makeCallback(callback);

  if (permission.isEnabled()) {
    callback(new ERR_ACCESS_DENIED('fchmod API is disabled when Permission Model is enabled.'));
    return;
  }

  const req = new FSReqCallback();
  req.oncomplete = callback;
  binding.fchmod(fd, mode, req);
}
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
  validateInteger(uid, 'uid', -1, kMaxUserId);
  validateInteger(gid, 'gid', -1, kMaxUserId);
  callback = makeCallback(callback);
  if (permission.isEnabled()) {
    callback(new ERR_ACCESS_DENIED('fchown API is disabled when Permission Model is enabled.'));
    return;
  }

  const req = new FSReqCallback();
  req.oncomplete = callback;
  binding.fchown(fd, uid, gid, req);
function fchownSync(fd, uid, gid) {
  validateInteger(uid, 'uid', -1, kMaxUserId);
  validateInteger(gid, 'gid', -1, kMaxUserId);
  if (permission.isEnabled()) {
    throw new ERR_ACCESS_DENIED('fchown API is disabled when Permission Model is enabled.');
  }

  binding.fchown(fd, uid, gid);
}
