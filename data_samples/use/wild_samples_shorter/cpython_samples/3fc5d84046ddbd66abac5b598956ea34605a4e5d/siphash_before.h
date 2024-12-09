 * --------------------------------------------------------------------------
 * HISTORY:
 *
 * 2019-08-03  (Sebastian Pipping)
 *   - Mark part of sip24_valid as to be excluded from clang-format
 *   - Re-format code using clang-format 9
 *
#define SIPHASH_H

#include <stddef.h> /* size_t */

#if defined(_WIN32) && defined(_MSC_VER) && (_MSC_VER < 1600)
/* For vs2003/7.1 up to vs2008/9.0; _MSC_VER 1600 is vs2010/10.0 */
typedef unsigned __int8 uint8_t;
typedef unsigned __int32 uint32_t;
typedef unsigned __int64 uint64_t;
#else
#  include <stdint.h> /* uint64_t uint32_t uint8_t */
#endif

/*
 * Workaround to not require a C++11 compiler for using ULL suffix
 * if this code is included and compiled as C++; related GCC warning is: