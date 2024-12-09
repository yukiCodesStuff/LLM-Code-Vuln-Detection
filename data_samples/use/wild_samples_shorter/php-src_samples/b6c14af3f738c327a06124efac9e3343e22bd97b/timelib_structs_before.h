
#include "timelib_config.h"

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#if defined(HAVE_INTTYPES_H)
#include <inttypes.h>
#elif defined(HAVE_STDINT_H)
#include <stdint.h>
#endif

# ifndef HAVE_INT32_T
#  if SIZEOF_INT == 4
typedef int int32_t;
#  elif SIZEOF_LONG == 4
typedef long int int32_t;
#  endif
# endif

# ifndef HAVE_UINT32_T
#  if SIZEOF_INT == 4
typedef unsigned int uint32_t;
#  elif SIZEOF_LONG == 4
typedef unsigned long int uint32_t;
#  endif
# endif

#include <stdio.h>

#ifdef HAVE_STDLIB_H
#endif

#if defined(_MSC_VER)
typedef __uint64 timelib_ull;
typedef __int64 timelib_sll;
# define TIMELIB_LL_CONST(n) n ## i64
#else
typedef unsigned long long timelib_ull;
typedef signed long long timelib_sll;