const IDX_OPTIONS_MAX_OUTSTANDING_PINGS = 6;
const IDX_OPTIONS_MAX_OUTSTANDING_SETTINGS = 7;
const IDX_OPTIONS_MAX_SESSION_MEMORY = 8;
const IDX_OPTIONS_FLAGS = 9;

function updateOptionsBuffer(options) {
  let flags = 0;
  if (typeof options.maxDeflateDynamicTableSize === 'number') {
    optionsBuffer[IDX_OPTIONS_MAX_SESSION_MEMORY] =
      MathMax(1, options.maxSessionMemory);
  }
  optionsBuffer[IDX_OPTIONS_FLAGS] = flags;
}

function getDefaultSettings() {