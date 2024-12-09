'use strict';

const { setUnrefTimeout } = require('internal/timers');

var nowCache;
var utcCache;

function nowDate() {
  if (!nowCache) cache();
  return nowCache;
}
module.exports = {
  outHeadersKey: Symbol('outHeadersKey'),
  ondrain,
  nowDate,
  utcDate
};