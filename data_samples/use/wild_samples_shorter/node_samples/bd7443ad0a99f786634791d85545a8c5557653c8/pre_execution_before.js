  ERR_MANIFEST_ASSERT_INTEGRITY,
  ERR_NO_CRYPTO,
  ERR_MISSING_OPTION,
} = require('internal/errors').codes;
const assert = require('internal/assert');
const {
  namespace: {
function initializePermission() {
  const experimentalPermission = getOptionValue('--experimental-permission');
  if (experimentalPermission) {
    process.emitWarning('Permission is an experimental feature',
                        'ExperimentalWarning');
    const { has, deny } = require('internal/process/permission');
    const warnFlags = [