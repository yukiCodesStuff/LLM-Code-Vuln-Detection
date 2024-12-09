
'use strict';

const { toASCII } = internalBinding('config').hasIntl ?
  internalBinding('icu') : require('punycode');

const { hexTable } = require('internal/querystring');

const { SafeSet } = require('internal/safe_globals');

const {
  ERR_INVALID_ARG_TYPE