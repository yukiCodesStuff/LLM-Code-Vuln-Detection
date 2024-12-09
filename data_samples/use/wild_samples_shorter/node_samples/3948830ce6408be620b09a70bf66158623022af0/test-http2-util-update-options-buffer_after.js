const IDX_OPTIONS_MAX_OUTSTANDING_PINGS = 6;
const IDX_OPTIONS_MAX_OUTSTANDING_SETTINGS = 7;
const IDX_OPTIONS_MAX_SESSION_MEMORY = 8;
const IDX_OPTIONS_MAX_SETTINGS = 9;
const IDX_OPTIONS_FLAGS = 10;

{
  updateOptionsBuffer({
    maxDeflateDynamicTableSize: 1,
    maxHeaderListPairs: 6,
    maxOutstandingPings: 7,
    maxOutstandingSettings: 8,
    maxSessionMemory: 9,
    maxSettings: 10,
  });

  strictEqual(optionsBuffer[IDX_OPTIONS_MAX_DEFLATE_DYNAMIC_TABLE_SIZE], 1);
  strictEqual(optionsBuffer[IDX_OPTIONS_MAX_RESERVED_REMOTE_STREAMS], 2);
  strictEqual(optionsBuffer[IDX_OPTIONS_MAX_OUTSTANDING_PINGS], 7);
  strictEqual(optionsBuffer[IDX_OPTIONS_MAX_OUTSTANDING_SETTINGS], 8);
  strictEqual(optionsBuffer[IDX_OPTIONS_MAX_SESSION_MEMORY], 9);
  strictEqual(optionsBuffer[IDX_OPTIONS_MAX_SETTINGS], 10);

  const flags = optionsBuffer[IDX_OPTIONS_FLAGS];

  ok(flags & (1 << IDX_OPTIONS_MAX_DEFLATE_DYNAMIC_TABLE_SIZE));
  ok(flags & (1 << IDX_OPTIONS_MAX_HEADER_LIST_PAIRS));
  ok(flags & (1 << IDX_OPTIONS_MAX_OUTSTANDING_PINGS));
  ok(flags & (1 << IDX_OPTIONS_MAX_OUTSTANDING_SETTINGS));
  ok(flags & (1 << IDX_OPTIONS_MAX_SETTINGS));
}

{
  optionsBuffer[IDX_OPTIONS_MAX_SEND_HEADER_BLOCK_LENGTH] = 0;