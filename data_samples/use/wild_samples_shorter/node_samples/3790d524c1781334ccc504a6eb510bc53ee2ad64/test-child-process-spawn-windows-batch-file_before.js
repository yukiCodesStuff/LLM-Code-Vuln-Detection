const assert = require('assert');
const { isWindows } = common;

const arg = '--security-revert=CVE-2024-27980';
const isRevert = process.execArgv.includes(arg);

const expectedCode = isWindows && !isRevert ? 'EINVAL' : 'ENOENT';
const expectedStatus = isWindows ? 1 : 127;

const suffixes =
    'BAT bAT BaT baT BAt bAt Bat bat CMD cMD CmD cmD CMd cMd Cmd cmd'
    .split(' ');

if (process.argv[2] === undefined) {
  const a = cp.spawnSync(process.execPath, [__filename, 'child']);
  const b = cp.spawnSync(process.execPath, [arg, __filename, 'child']);
  assert.strictEqual(a.status, 0);
  assert.strictEqual(b.status, 0);
  return;
}

function testExec(filename) {
  return new Promise((resolve) => {
    cp.exec(filename).once('exit', common.mustCall(function(status) {
      assert.strictEqual(status, expectedStatus);