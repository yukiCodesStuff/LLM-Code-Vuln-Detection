 */

#define PLATFORM "platform: linux64-s390x"
#define DATE "built on: Fri Jul  8 16:49:20 2022 UTC"

/*
 * Generate compiler_flags as an array of individual characters. This is a
 * workaround for the situation where CFLAGS gets too long for a C90 string