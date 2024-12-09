                        \___/_/\_\ .__/ \__,_|\__|
                                 |_| XML parser

   Copyright (c) 2002-2003 Fred L. Drake, Jr. <fdrake@users.sourceforge.net>
   Copyright (c) 2002-2006 Karl Waclawek <karl@waclawek.net>
   Copyright (c) 2003      Greg Stein <gstein@users.sourceforge.net>
   Copyright (c) 2016-2021 Sebastian Pipping <sebastian@pipping.org>
   Copyright (c) 2018      Yury Gribov <tetra2005@gmail.com>
   Copyright (c) 2019      David Loffredo <loffredo@steptools.com>
   Licensed under the MIT license:

   Permission is  hereby granted,  free of charge,  to any  person obtaining
   a  copy  of  this  software   and  associated  documentation  files  (the
#  endif
#endif

#include <limits.h> // ULONG_MAX

#if defined(_WIN32) && ! defined(__USE_MINGW_ANSI_STDIO)
#  define EXPAT_FMT_ULL(midpart) "%" midpart "I64u"
#  if defined(_WIN64) // Note: modifiers "td" and "zu" do not work for MinGW
#    define EXPAT_FMT_PTRDIFF_T(midpart) "%" midpart "I64d"
#    define EXPAT_FMT_SIZE_T(midpart) "%" midpart "I64u"
#  else
#    define EXPAT_FMT_PTRDIFF_T(midpart) "%" midpart "d"
#    define EXPAT_FMT_SIZE_T(midpart) "%" midpart "u"
#  endif
#else
#  define EXPAT_FMT_ULL(midpart) "%" midpart "llu"
#  if ! defined(ULONG_MAX)
#    error Compiler did not define ULONG_MAX for us
#  elif ULONG_MAX == 18446744073709551615u // 2^64-1
#    define EXPAT_FMT_PTRDIFF_T(midpart) "%" midpart "ld"
#    define EXPAT_FMT_SIZE_T(midpart) "%" midpart "lu"
#  else
#    define EXPAT_FMT_PTRDIFF_T(midpart) "%" midpart "d"
#    define EXPAT_FMT_SIZE_T(midpart) "%" midpart "u"
#  endif
#endif

#ifndef UNUSED_P
#  define UNUSED_P(p) (void)p
#endif

/* NOTE BEGIN If you ever patch these defaults to greater values
              for non-attack XML payload in your environment,
              please file a bug report with libexpat.  Thank you!
*/
#define EXPAT_BILLION_LAUGHS_ATTACK_PROTECTION_MAXIMUM_AMPLIFICATION_DEFAULT   \
  100.0f
#define EXPAT_BILLION_LAUGHS_ATTACK_PROTECTION_ACTIVATION_THRESHOLD_DEFAULT    \
  8388608 // 8 MiB, 2^23
/* NOTE END */

#include "expat.h" // so we can use type XML_Parser below

#ifdef __cplusplus
extern "C" {
#endif

void _INTERNAL_trim_to_complete_utf8_characters(const char *from,
                                                const char **fromLimRef);

#if defined(XML_DTD)
unsigned long long testingAccountingGetCountBytesDirect(XML_Parser parser);
unsigned long long testingAccountingGetCountBytesIndirect(XML_Parser parser);
const char *unsignedCharToPrintable(unsigned char c);
#endif

#ifdef __cplusplus
}
#endif