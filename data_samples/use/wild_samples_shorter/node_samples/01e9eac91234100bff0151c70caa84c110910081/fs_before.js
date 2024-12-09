  mode = parseFileMode(mode, 'mode');
  callback = makeCallback(callback);

  const req = new FSReqCallback();
  req.oncomplete = callback;
  binding.fchmod(fd, mode, req);
}
 * @returns {void}
 */
function fchmodSync(fd, mode) {
  binding.fchmod(
    fd,
    parseFileMode(mode, 'mode'),
  );
  validateInteger(uid, 'uid', -1, kMaxUserId);
  validateInteger(gid, 'gid', -1, kMaxUserId);
  callback = makeCallback(callback);

  const req = new FSReqCallback();
  req.oncomplete = callback;
  binding.fchown(fd, uid, gid, req);
function fchownSync(fd, uid, gid) {
  validateInteger(uid, 'uid', -1, kMaxUserId);
  validateInteger(gid, 'gid', -1, kMaxUserId);

  binding.fchown(fd, uid, gid);
}
