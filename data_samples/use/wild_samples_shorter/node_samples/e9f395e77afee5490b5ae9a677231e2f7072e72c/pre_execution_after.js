  DatePrototypeGetSeconds,
  NumberParseInt,
  ObjectDefineProperty,
  ObjectFreeze,
  ObjectGetOwnPropertyDescriptor,
  SafeMap,
  String,
  StringPrototypeStartsWith,
    process.binding = function binding(_module) {
      throw new ERR_ACCESS_DENIED('process.binding');
    };
    // Guarantee path module isn't monkey-patched to bypass permission model
    ObjectFreeze(require('path'));
    process.emitWarning('Permission is an experimental feature',
                        'ExperimentalWarning');
    const { has, deny } = require('internal/process/permission');
    const warnFlags = [