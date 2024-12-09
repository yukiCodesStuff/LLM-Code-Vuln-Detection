 * --------------------------------------------------------------------------
 * HISTORY:
 *
 * 2020-10-03  (Sebastian Pipping)
 *   - Drop support for Visual Studio 9.0/2008 and earlier
 *
 * 2019-08-03  (Sebastian Pipping)
 *   - Mark part of sip24_valid as to be excluded from clang-format
 *   - Re-format code using clang-format 9
 *
#define SIPHASH_H

#include <stddef.h> /* size_t */
#include <stdint.h> /* uint64_t uint32_t uint8_t */

/*
 * Workaround to not require a C++11 compiler for using ULL suffix
 * if this code is included and compiled as C++; related GCC warning is: