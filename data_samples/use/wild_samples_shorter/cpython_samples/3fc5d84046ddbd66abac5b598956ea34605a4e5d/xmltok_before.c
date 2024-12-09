                                 |_| XML parser

   Copyright (c) 1997-2000 Thai Open Source Software Center Ltd
   Copyright (c) 2000-2017 Expat development team
   Licensed under the MIT license:

   Permission is  hereby granted,  free of charge,  to any  person obtaining
   a  copy  of  this  software   and  associated  documentation  files  (the
   USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifdef _WIN32
#  include "winconfig.h"
#else
#  ifdef HAVE_EXPAT_CONFIG_H
#    include <expat_config.h>
#  endif
#endif /* ndef _WIN32 */

#include <stddef.h>
#include <string.h> /* memcpy */

#if defined(_MSC_VER) && (_MSC_VER <= 1700)
/* for vs2012/11.0/1700 and earlier Visual Studio compilers */
#  define bool int
#  define false 0
#  define true 1
#else
#  include <stdbool.h>
#endif

#include "expat_external.h"
#include "internal.h"
#include "xmltok.h"
#include "nametab.h"

#define IS_NAME_CHAR(enc, p, n) (AS_NORMAL_ENCODING(enc)->isName##n(enc, p))
#define IS_NMSTRT_CHAR(enc, p, n) (AS_NORMAL_ENCODING(enc)->isNmstrt##n(enc, p))
#define IS_INVALID_CHAR(enc, p, n)                                             \
  (AS_NORMAL_ENCODING(enc)->isInvalid##n(enc, p))

#ifdef XML_MIN_SIZE
#  define IS_NAME_CHAR_MINBPC(enc, p)                                          \
    (AS_NORMAL_ENCODING(enc)->isNameMin(enc, p))
static int PTRFASTCALL
unicode_byte_type(char hi, char lo) {
  switch ((unsigned char)hi) {
  /* 0xD800–0xDBFF first 16-bit code unit or high surrogate (W1) */
  case 0xD8:
  case 0xD9:
  case 0xDA:
  case 0xDB:
    return BT_LEAD4;
  /* 0xDC00–0xDFFF second 16-bit code unit or low surrogate (W2) */
  case 0xDC:
  case 0xDD:
  case 0xDE:
  case 0xDF: