const expectedStatus = isWindows ? 1 : 127;

const suffixes =
    'BAT bAT BaT baT BAt bAt Bat bat CMD cMD CmD cmD CMd cMd Cmd cmd'
    .split(' ');

function testExec(filename) {
  return new Promise((resolve) => {
    cp.exec(filename).once('exit', common.mustCall(function(status) {