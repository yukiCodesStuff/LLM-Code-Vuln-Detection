  return 0;
}

TEST_IMPL(utf8_decode1_overrun) {
  const char* p;
  char b[1];

  /* Single byte. */
  p = b;
  b[0] = 0x7F;
  ASSERT_EQ(0x7F, uv__utf8_decode1(&p, b + 1));
  ASSERT_EQ(p, b + 1);

  /* Multi-byte. */
  p = b;
  b[0] = 0xC0;
  ASSERT_EQ((unsigned) -1, uv__utf8_decode1(&p, b + 1));
  ASSERT_EQ(p, b + 1);

  return 0;
}

/* Doesn't work on z/OS because that platform uses EBCDIC, not ASCII. */
#ifndef __MVS__

#define F(input, err)                                                         \