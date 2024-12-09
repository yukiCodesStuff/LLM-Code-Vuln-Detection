/*
                            __  __            _
                         ___\ \/ /_ __   __ _| |_
                        / _ \\  /| '_ \ / _` | __|
                       |  __//  \| |_) | (_| | |_
                        \___/_/\_\ .__/ \__,_|\__|
                                 |_| XML parser

   Copyright (c) 1997-2000 Thai Open Source Software Center Ltd
   Copyright (c) 2000      Clark Cooper <coopercc@users.sourceforge.net>
   Copyright (c) 2001-2003 Fred L. Drake, Jr. <fdrake@users.sourceforge.net>
   Copyright (c) 2002      Greg Stein <gstein@users.sourceforge.net>
   Copyright (c) 2002-2016 Karl Waclawek <karl@waclawek.net>
   Copyright (c) 2005-2009 Steven Solie <ssolie@users.sourceforge.net>
   Copyright (c) 2016-2021 Sebastian Pipping <sebastian@pipping.org>
   Copyright (c) 2016      Pascal Cuoq <cuoq@trust-in-soft.com>
   Copyright (c) 2016      Don Lewis <truckman@apache.org>
   Copyright (c) 2017      Rhodri James <rhodri@wildebeest.org.uk>
   Copyright (c) 2017      Alexander Bluhm <alexander.bluhm@gmx.net>
   Copyright (c) 2017      Benbuck Nason <bnason@netflix.com>
   Copyright (c) 2017      José Gutiérrez de la Concha <jose@zeroc.com>
   Copyright (c) 2019      David Loffredo <loffredo@steptools.com>
   Licensed under the MIT license:

   Permission is  hereby granted,  free of charge,  to any  person obtaining
   a  copy  of  this  software   and  associated  documentation  files  (the
   "Software"),  to  deal in  the  Software  without restriction,  including
   without  limitation the  rights  to use,  copy,  modify, merge,  publish,
   distribute, sublicense, and/or sell copies of the Software, and to permit
   persons  to whom  the Software  is  furnished to  do so,  subject to  the
   following conditions:

   The above copyright  notice and this permission notice  shall be included
   in all copies or substantial portions of the Software.

   THE  SOFTWARE  IS  PROVIDED  "AS  IS",  WITHOUT  WARRANTY  OF  ANY  KIND,
   EXPRESS  OR IMPLIED,  INCLUDING  BUT  NOT LIMITED  TO  THE WARRANTIES  OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
   NO EVENT SHALL THE AUTHORS OR  COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
   DAMAGES OR  OTHER LIABILITY, WHETHER  IN AN  ACTION OF CONTRACT,  TORT OR
   OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
   USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <stddef.h>
#include <string.h> /* memcpy */
#include <stdbool.h>

#ifdef _WIN32
#  include "winconfig.h"
#endif

#include <expat_config.h>

#include "expat_external.h"
#include "internal.h"
#include "xmltok.h"
#include "nametab.h"

#ifdef XML_DTD
#  define IGNORE_SECTION_TOK_VTABLE , PREFIX(ignoreSectionTok)
#else
#  define IGNORE_SECTION_TOK_VTABLE /* as nothing */
#endif

#define VTABLE1                                                                \
  {PREFIX(prologTok), PREFIX(contentTok),                                      \
   PREFIX(cdataSectionTok) IGNORE_SECTION_TOK_VTABLE},                         \
      {PREFIX(attributeValueTok), PREFIX(entityValueTok)},                     \
      PREFIX(nameMatchesAscii), PREFIX(nameLength), PREFIX(skipS),             \
      PREFIX(getAtts), PREFIX(charRefNumber), PREFIX(predefinedEntityName),    \
      PREFIX(updatePosition), PREFIX(isPublicId)

#define VTABLE VTABLE1, PREFIX(toUtf8), PREFIX(toUtf16)

#define UCS2_GET_NAMING(pages, hi, lo)                                         \
  (namingBitmap[(pages[hi] << 3) + ((lo) >> 5)] & (1u << ((lo)&0x1F)))

/* A 2 byte UTF-8 representation splits the characters 11 bits between
   the bottom 5 and 6 bits of the bytes.  We need 8 bits to index into
   pages, 3 bits to add to that index and 5 bits to generate the mask.
*/
#define UTF8_GET_NAMING2(pages, byte)                                          \
  (namingBitmap[((pages)[(((byte)[0]) >> 2) & 7] << 3)                         \
                + ((((byte)[0]) & 3) << 1) + ((((byte)[1]) >> 5) & 1)]         \
   & (1u << (((byte)[1]) & 0x1F)))

/* A 3 byte UTF-8 representation splits the characters 16 bits between
   the bottom 4, 6 and 6 bits of the bytes.  We need 8 bits to index
   into pages, 3 bits to add to that index and 5 bits to generate the
   mask.
*/
#define UTF8_GET_NAMING3(pages, byte)                                          \
  (namingBitmap                                                                \
       [((pages)[((((byte)[0]) & 0xF) << 4) + ((((byte)[1]) >> 2) & 0xF)]      \
         << 3)                                                                 \
        + ((((byte)[1]) & 3) << 1) + ((((byte)[2]) >> 5) & 1)]                 \
   & (1u << (((byte)[2]) & 0x1F)))

#define UTF8_GET_NAMING(pages, p, n)                                           \
  ((n) == 2                                                                    \
       ? UTF8_GET_NAMING2(pages, (const unsigned char *)(p))                   \
       : ((n) == 3 ? UTF8_GET_NAMING3(pages, (const unsigned char *)(p)) : 0))

/* Detection of invalid UTF-8 sequences is based on Table 3.1B
   of Unicode 3.2: http://www.unicode.org/unicode/reports/tr28/
   with the additional restriction of not allowing the Unicode
   code points 0xFFFF and 0xFFFE (sequences EF,BF,BF and EF,BF,BE).
   Implementation details:
     (A & 0x80) == 0     means A < 0x80
   and
     (A & 0xC0) == 0xC0  means A > 0xBF
*/

#define UTF8_INVALID2(p)                                                       \
  ((*p) < 0xC2 || ((p)[1] & 0x80) == 0 || ((p)[1] & 0xC0) == 0xC0)

#define UTF8_INVALID3(p)                                                       \
  (((p)[2] & 0x80) == 0                                                        \
   || ((*p) == 0xEF && (p)[1] == 0xBF ? (p)[2] > 0xBD                          \
                                      : ((p)[2] & 0xC0) == 0xC0)               \
   || ((*p) == 0xE0                                                            \
           ? (p)[1] < 0xA0 || ((p)[1] & 0xC0) == 0xC0                          \
           : ((p)[1] & 0x80) == 0                                              \
                 || ((*p) == 0xED ? (p)[1] > 0x9F : ((p)[1] & 0xC0) == 0xC0)))

#define UTF8_INVALID4(p)                                                       \
  (((p)[3] & 0x80) == 0 || ((p)[3] & 0xC0) == 0xC0 || ((p)[2] & 0x80) == 0     \
   || ((p)[2] & 0xC0) == 0xC0                                                  \
   || ((*p) == 0xF0                                                            \
           ? (p)[1] < 0x90 || ((p)[1] & 0xC0) == 0xC0                          \
           : ((p)[1] & 0x80) == 0                                              \
                 || ((*p) == 0xF4 ? (p)[1] > 0x8F : ((p)[1] & 0xC0) == 0xC0)))

static int PTRFASTCALL
isNever(const ENCODING *enc, const char *p) {
  UNUSED_P(enc);
  UNUSED_P(p);
  return 0;
}
/*
                            __  __            _
                         ___\ \/ /_ __   __ _| |_
                        / _ \\  /| '_ \ / _` | __|
                       |  __//  \| |_) | (_| | |_
                        \___/_/\_\ .__/ \__,_|\__|
                                 |_| XML parser

   Copyright (c) 1997-2000 Thai Open Source Software Center Ltd
   Copyright (c) 2000      Clark Cooper <coopercc@users.sourceforge.net>
   Copyright (c) 2001-2003 Fred L. Drake, Jr. <fdrake@users.sourceforge.net>
   Copyright (c) 2002      Greg Stein <gstein@users.sourceforge.net>
   Copyright (c) 2002-2016 Karl Waclawek <karl@waclawek.net>
   Copyright (c) 2005-2009 Steven Solie <ssolie@users.sourceforge.net>
   Copyright (c) 2016-2021 Sebastian Pipping <sebastian@pipping.org>
   Copyright (c) 2016      Pascal Cuoq <cuoq@trust-in-soft.com>
   Copyright (c) 2016      Don Lewis <truckman@apache.org>
   Copyright (c) 2017      Rhodri James <rhodri@wildebeest.org.uk>
   Copyright (c) 2017      Alexander Bluhm <alexander.bluhm@gmx.net>
   Copyright (c) 2017      Benbuck Nason <bnason@netflix.com>
   Copyright (c) 2017      José Gutiérrez de la Concha <jose@zeroc.com>
   Copyright (c) 2019      David Loffredo <loffredo@steptools.com>
   Licensed under the MIT license:

   Permission is  hereby granted,  free of charge,  to any  person obtaining
   a  copy  of  this  software   and  associated  documentation  files  (the
   "Software"),  to  deal in  the  Software  without restriction,  including
   without  limitation the  rights  to use,  copy,  modify, merge,  publish,
   distribute, sublicense, and/or sell copies of the Software, and to permit
   persons  to whom  the Software  is  furnished to  do so,  subject to  the
   following conditions:

   The above copyright  notice and this permission notice  shall be included
   in all copies or substantial portions of the Software.

   THE  SOFTWARE  IS  PROVIDED  "AS  IS",  WITHOUT  WARRANTY  OF  ANY  KIND,
   EXPRESS  OR IMPLIED,  INCLUDING  BUT  NOT LIMITED  TO  THE WARRANTIES  OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
   NO EVENT SHALL THE AUTHORS OR  COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
   DAMAGES OR  OTHER LIABILITY, WHETHER  IN AN  ACTION OF CONTRACT,  TORT OR
   OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
   USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <stddef.h>
#include <string.h> /* memcpy */
#include <stdbool.h>

#ifdef _WIN32
#  include "winconfig.h"
#endif

#include <expat_config.h>

#include "expat_external.h"
#include "internal.h"
#include "xmltok.h"
#include "nametab.h"

#ifdef XML_DTD
#  define IGNORE_SECTION_TOK_VTABLE , PREFIX(ignoreSectionTok)
#else
#  define IGNORE_SECTION_TOK_VTABLE /* as nothing */
#endif

#define VTABLE1                                                                \
  {PREFIX(prologTok), PREFIX(contentTok),                                      \
   PREFIX(cdataSectionTok) IGNORE_SECTION_TOK_VTABLE},                         \
      {PREFIX(attributeValueTok), PREFIX(entityValueTok)},                     \
      PREFIX(nameMatchesAscii), PREFIX(nameLength), PREFIX(skipS),             \
      PREFIX(getAtts), PREFIX(charRefNumber), PREFIX(predefinedEntityName),    \
      PREFIX(updatePosition), PREFIX(isPublicId)

#define VTABLE VTABLE1, PREFIX(toUtf8), PREFIX(toUtf16)

#define UCS2_GET_NAMING(pages, hi, lo)                                         \
  (namingBitmap[(pages[hi] << 3) + ((lo) >> 5)] & (1u << ((lo)&0x1F)))

/* A 2 byte UTF-8 representation splits the characters 11 bits between
   the bottom 5 and 6 bits of the bytes.  We need 8 bits to index into
   pages, 3 bits to add to that index and 5 bits to generate the mask.
*/
#define UTF8_GET_NAMING2(pages, byte)                                          \
  (namingBitmap[((pages)[(((byte)[0]) >> 2) & 7] << 3)                         \
                + ((((byte)[0]) & 3) << 1) + ((((byte)[1]) >> 5) & 1)]         \
   & (1u << (((byte)[1]) & 0x1F)))

/* A 3 byte UTF-8 representation splits the characters 16 bits between
   the bottom 4, 6 and 6 bits of the bytes.  We need 8 bits to index
   into pages, 3 bits to add to that index and 5 bits to generate the
   mask.
*/
#define UTF8_GET_NAMING3(pages, byte)                                          \
  (namingBitmap                                                                \
       [((pages)[((((byte)[0]) & 0xF) << 4) + ((((byte)[1]) >> 2) & 0xF)]      \
         << 3)                                                                 \
        + ((((byte)[1]) & 3) << 1) + ((((byte)[2]) >> 5) & 1)]                 \
   & (1u << (((byte)[2]) & 0x1F)))

#define UTF8_GET_NAMING(pages, p, n)                                           \
  ((n) == 2                                                                    \
       ? UTF8_GET_NAMING2(pages, (const unsigned char *)(p))                   \
       : ((n) == 3 ? UTF8_GET_NAMING3(pages, (const unsigned char *)(p)) : 0))

/* Detection of invalid UTF-8 sequences is based on Table 3.1B
   of Unicode 3.2: http://www.unicode.org/unicode/reports/tr28/
   with the additional restriction of not allowing the Unicode
   code points 0xFFFF and 0xFFFE (sequences EF,BF,BF and EF,BF,BE).
   Implementation details:
     (A & 0x80) == 0     means A < 0x80
   and
     (A & 0xC0) == 0xC0  means A > 0xBF
*/

#define UTF8_INVALID2(p)                                                       \
  ((*p) < 0xC2 || ((p)[1] & 0x80) == 0 || ((p)[1] & 0xC0) == 0xC0)

#define UTF8_INVALID3(p)                                                       \
  (((p)[2] & 0x80) == 0                                                        \
   || ((*p) == 0xEF && (p)[1] == 0xBF ? (p)[2] > 0xBD                          \
                                      : ((p)[2] & 0xC0) == 0xC0)               \
   || ((*p) == 0xE0                                                            \
           ? (p)[1] < 0xA0 || ((p)[1] & 0xC0) == 0xC0                          \
           : ((p)[1] & 0x80) == 0                                              \
                 || ((*p) == 0xED ? (p)[1] > 0x9F : ((p)[1] & 0xC0) == 0xC0)))

#define UTF8_INVALID4(p)                                                       \
  (((p)[3] & 0x80) == 0 || ((p)[3] & 0xC0) == 0xC0 || ((p)[2] & 0x80) == 0     \
   || ((p)[2] & 0xC0) == 0xC0                                                  \
   || ((*p) == 0xF0                                                            \
           ? (p)[1] < 0x90 || ((p)[1] & 0xC0) == 0xC0                          \
           : ((p)[1] & 0x80) == 0                                              \
                 || ((*p) == 0xF4 ? (p)[1] > 0x8F : ((p)[1] & 0xC0) == 0xC0)))

static int PTRFASTCALL
isNever(const ENCODING *enc, const char *p) {
  UNUSED_P(enc);
  UNUSED_P(p);
  return 0;
}
sb_byteToAscii(const ENCODING *enc, const char *p) {
  UNUSED_P(enc);
  return *p;
}
#else
#  define BYTE_TO_ASCII(enc, p) (*(p))
#endif

#define IS_NAME_CHAR(enc, p, n) (AS_NORMAL_ENCODING(enc)->isName##n(enc, p))
#define IS_NMSTRT_CHAR(enc, p, n) (AS_NORMAL_ENCODING(enc)->isNmstrt##n(enc, p))
#ifdef XML_MIN_SIZE
#  define IS_INVALID_CHAR(enc, p, n)                                           \
    (AS_NORMAL_ENCODING(enc)->isInvalid##n                                     \
     && AS_NORMAL_ENCODING(enc)->isInvalid##n(enc, p))
#else
#  define IS_INVALID_CHAR(enc, p, n)                                           \
    (AS_NORMAL_ENCODING(enc)->isInvalid##n(enc, p))
#endif

#ifdef XML_MIN_SIZE
#  define IS_NAME_CHAR_MINBPC(enc, p)                                          \
    (AS_NORMAL_ENCODING(enc)->isNameMin(enc, p))
#  define IS_NMSTRT_CHAR_MINBPC(enc, p)                                        \
    (AS_NORMAL_ENCODING(enc)->isNmstrtMin(enc, p))
#else
#  define IS_NAME_CHAR_MINBPC(enc, p) (0)
#  define IS_NMSTRT_CHAR_MINBPC(enc, p) (0)
#endif

#ifdef XML_MIN_SIZE
#  define CHAR_MATCHES(enc, p, c)                                              \
    (AS_NORMAL_ENCODING(enc)->charMatches(enc, p, c))
static int PTRCALL
sb_charMatches(const ENCODING *enc, const char *p, int c) {
  UNUSED_P(enc);
  return *p == c;
}
unicode_byte_type(char hi, char lo) {
  switch ((unsigned char)hi) {
  /* 0xD800-0xDBFF first 16-bit code unit or high surrogate (W1) */
  case 0xD8:
  case 0xD9:
  case 0xDA:
  case 0xDB:
    return BT_LEAD4;
  /* 0xDC00-0xDFFF second 16-bit code unit or low surrogate (W2) */
  case 0xDC:
  case 0xDD:
  case 0xDE:
  case 0xDF:
    return BT_TRAIL;
  case 0xFF:
    switch ((unsigned char)lo) {
    case 0xFF: /* noncharacter-FFFF */
    case 0xFE: /* noncharacter-FFFE */
      return BT_NONXML;
    }
    break;
  }
  return BT_NONASCII;
}