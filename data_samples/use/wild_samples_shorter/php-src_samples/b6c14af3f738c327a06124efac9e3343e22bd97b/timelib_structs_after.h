
#include "timelib_config.h"

#include "php_stdint.h"

#include <stdio.h>

#ifdef HAVE_STDLIB_H
#endif

#if defined(_MSC_VER)
typedef uint64_t timelib_ull;
typedef int64_t timelib_sll;
# define TIMELIB_LL_CONST(n) n ## i64
#else
typedef unsigned long long timelib_ull;
typedef signed long long timelib_sll;