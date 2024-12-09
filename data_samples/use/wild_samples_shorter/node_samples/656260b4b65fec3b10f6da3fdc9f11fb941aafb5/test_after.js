assert.throws(() => {
  test_string.TestLargeUtf16();
}, /^Error: Invalid argument$/);

test_string.TestMemoryCorruption(' '.repeat(64 * 1024));