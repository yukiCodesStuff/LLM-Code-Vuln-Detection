
TEST_DECLARE  (idna_toascii)
TEST_DECLARE  (utf8_decode1)
TEST_DECLARE  (uname)

TEST_DECLARE  (metrics_idle_time)
TEST_DECLARE  (metrics_idle_time_thread)
#endif

  TEST_ENTRY  (utf8_decode1)
  TEST_ENTRY  (uname)

/* Doesn't work on z/OS because that platform uses EBCDIC, not ASCII. */
#ifndef __MVS__