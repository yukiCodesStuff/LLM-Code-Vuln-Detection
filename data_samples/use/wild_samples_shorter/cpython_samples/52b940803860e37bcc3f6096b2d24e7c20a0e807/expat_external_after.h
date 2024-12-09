
/* External API definitions */

/* Expat tries very hard to make the API boundary very specifically
   defined.  There are two macros defined to control this boundary;
   each of these can be defined before including this header to
   achieve some different behavior, but doing so it not recommended or
   compiled with the cdecl calling convention as the default since
   system headers may assume the cdecl convention.
*/

/* Namespace external symbols to allow multiple libexpat version to
   co-exist. */
#include "pyexpatns.h"

#ifndef XMLCALL
#  if defined(_MSC_VER)
#    define XMLCALL __cdecl
#  elif defined(__GNUC__) && defined(__i386) && ! defined(__INTEL_COMPILER)
#    define XMLCALL __attribute__((cdecl))
#  else
/* For any platform which uses this definition and supports more than
   one calling convention, we need to extend this definition to
   declare the convention used on that platform, if it's possible to
   do so.
   pre-processor and how to specify the same calling convention as the
   platform's malloc() implementation.
*/
#    define XMLCALL
#  endif
#endif /* not defined XMLCALL */

#if ! defined(XML_STATIC) && ! defined(XMLIMPORT)
#  ifndef XML_BUILDING_EXPAT
/* using Expat from an application */

#    if defined(_MSC_EXTENSIONS) && ! defined(__BEOS__) && ! defined(__CYGWIN__)
#      define XMLIMPORT __declspec(dllimport)
#    endif

#  endif
#endif /* not defined XML_STATIC */

#ifndef XML_ENABLE_VISIBILITY
#  define XML_ENABLE_VISIBILITY 0
#endif

#if ! defined(XMLIMPORT) && XML_ENABLE_VISIBILITY
#  define XMLIMPORT __attribute__((visibility("default")))
#endif

/* If we didn't define it above, define it away: */
#ifndef XMLIMPORT
#  define XMLIMPORT
#endif

#if defined(__GNUC__)                                                          \
    && (__GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 96))
#  define XML_ATTR_MALLOC __attribute__((__malloc__))
#else
#  define XML_ATTR_MALLOC
#endif

#if defined(__GNUC__)                                                          \
    && ((__GNUC__ > 4) || (__GNUC__ == 4 && __GNUC_MINOR__ >= 3))
#  define XML_ATTR_ALLOC_SIZE(x) __attribute__((__alloc_size__(x)))
#else
#  define XML_ATTR_ALLOC_SIZE(x)
#endif

#define XMLPARSEAPI(type) XMLIMPORT type XMLCALL

#endif

#ifdef XML_UNICODE_WCHAR_T
#  ifndef XML_UNICODE
#    define XML_UNICODE
#  endif
#  if defined(__SIZEOF_WCHAR_T__) && (__SIZEOF_WCHAR_T__ != 2)
#    error "sizeof(wchar_t) != 2; Need -fshort-wchar for both Expat and libc"
#  endif
#endif

#ifdef XML_UNICODE /* Information is UTF-16 encoded. */
#  ifdef XML_UNICODE_WCHAR_T
typedef wchar_t XML_Char;
typedef wchar_t XML_LChar;
#  else
typedef unsigned short XML_Char;
typedef char XML_LChar;
#  endif /* XML_UNICODE_WCHAR_T */
#else    /* Information is UTF-8 encoded. */
typedef char XML_Char;
typedef char XML_LChar;
#endif   /* XML_UNICODE */

#ifdef XML_LARGE_SIZE /* Use large integers for file/stream positions. */
typedef long long XML_Index;
typedef unsigned long long XML_Size;
#else
typedef long XML_Index;
typedef unsigned long XML_Size;
#endif /* XML_LARGE_SIZE */