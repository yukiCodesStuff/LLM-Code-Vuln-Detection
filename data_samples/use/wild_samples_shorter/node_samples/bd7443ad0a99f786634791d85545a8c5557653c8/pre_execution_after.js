  ERR_MANIFEST_ASSERT_INTEGRITY,
  ERR_NO_CRYPTO,
  ERR_MISSING_OPTION,
  ERR_ACCESS_DENIED,
} = require('internal/errors').codes;
const assert = require('internal/assert');
const {
  namespace: {
function initializePermission() {
  const experimentalPermission = getOptionValue('--experimental-permission');
  if (experimentalPermission) {
    process.binding = function binding(_module) {
      throw new ERR_ACCESS_DENIED('process.binding');
    };
    process.emitWarning('Permission is an experimental feature',
                        'ExperimentalWarning');
    const { has, deny } = require('internal/process/permission');
    const warnFlags = [