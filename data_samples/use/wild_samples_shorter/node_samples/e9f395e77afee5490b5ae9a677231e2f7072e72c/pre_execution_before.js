  DatePrototypeGetSeconds,
  NumberParseInt,
  ObjectDefineProperty,
  ObjectGetOwnPropertyDescriptor,
  SafeMap,
  String,
  StringPrototypeStartsWith,
    process.binding = function binding(_module) {
      throw new ERR_ACCESS_DENIED('process.binding');
    };
    process.emitWarning('Permission is an experimental feature',
                        'ExperimentalWarning');
    const { has, deny } = require('internal/process/permission');
    const warnFlags = [