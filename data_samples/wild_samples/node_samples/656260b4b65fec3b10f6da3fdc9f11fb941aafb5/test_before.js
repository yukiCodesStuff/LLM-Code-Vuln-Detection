assert.throws(() => {
  test_string.TestLargeUtf16();
}, /^Error: Invalid argument$/);