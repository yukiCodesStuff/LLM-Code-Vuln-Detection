'use strict';

const { setUnrefTimeout } = require('internal/timers');

var dateCache;
function utcDate() {
  if (!dateCache) {
    const d = new Date();
    dateCache = d.toUTCString();

    setUnrefTimeout(resetCache, 1000 - d.getMilliseconds());
  }
  return dateCache;
}
module.exports = {
  outHeadersKey: Symbol('outHeadersKey'),
  ondrain,
  utcDate
};