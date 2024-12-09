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
static int PTRFASTCALL
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