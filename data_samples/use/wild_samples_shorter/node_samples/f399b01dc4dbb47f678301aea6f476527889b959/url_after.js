
'use strict';

const { toASCII } = require('internal/idna');
const { hexTable } = require('internal/querystring');
const { SafeSet } = require('internal/safe_globals');

const {
  ERR_INVALID_ARG_TYPE