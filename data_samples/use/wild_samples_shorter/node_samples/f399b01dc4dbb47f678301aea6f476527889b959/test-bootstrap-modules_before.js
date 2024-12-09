
const isMainThread = common.isMainThread;
const kCoverageModuleCount = process.env.NODE_V8_COVERAGE ? 1 : 0;
const kMaxModuleCount = (isMainThread ? 63 : 85) + kCoverageModuleCount;

assert(list.length <= kMaxModuleCount,
       `Total length: ${list.length}\n` + list.join('\n')
);