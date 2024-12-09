 */

#define PLATFORM "platform: linux64-s390x"
#define DATE "built on: Thu Nov  3 02:18:56 2022 UTC"

/*
 * Generate compiler_flags as an array of individual characters. This is a
 * workaround for the situation where CFLAGS gets too long for a C90 string