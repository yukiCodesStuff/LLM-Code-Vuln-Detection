/* This file is included!
                            __  __            _
                         ___\ \/ /_ __   __ _| |_
                        / _ \\  /| '_ \ / _` | __|
                       |  __//  \| |_) | (_| | |_
                        \___/_/\_\ .__/ \__,_|\__|
                                 |_| XML parser

   Copyright (c) 1997-2000 Thai Open Source Software Center Ltd
   Copyright (c) 2000-2017 Expat development team
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

#ifdef XML_TOK_IMPL_C

#  ifndef IS_INVALID_CHAR
#    define IS_INVALID_CHAR(enc, ptr, n) (0)
#  endif

#  define INVALID_LEAD_CASE(n, ptr, nextTokPtr)                                \
  case BT_LEAD##n:                                                             \
    if (end - ptr < n)                                                         \
      return XML_TOK_PARTIAL_CHAR;                                             \
    if (IS_INVALID_CHAR(enc, ptr, n)) {                                        \
      *(nextTokPtr) = (ptr);                                                   \
      return XML_TOK_INVALID;                                                  \
    }                                                                          \
    ptr += n;                                                                  \
    break;

#  define INVALID_CASES(ptr, nextTokPtr)                                       \
    INVALID_LEAD_CASE(2, ptr, nextTokPtr)                                      \
    INVALID_LEAD_CASE(3, ptr, nextTokPtr)                                      \
    INVALID_LEAD_CASE(4, ptr, nextTokPtr)                                      \
  case BT_NONXML:                                                              \
  case BT_MALFORM:                                                             \
  case BT_TRAIL:                                                               \
    *(nextTokPtr) = (ptr);                                                     \
    return XML_TOK_INVALID;

#  define CHECK_NAME_CASE(n, enc, ptr, end, nextTokPtr)                        \
  case BT_LEAD##n:                                                             \
    if (end - ptr < n)                                                         \
      return XML_TOK_PARTIAL_CHAR;                                             \
    if (! IS_NAME_CHAR(enc, ptr, n)) {                                         \
      *nextTokPtr = ptr;                                                       \
      return XML_TOK_INVALID;                                                  \
    }                                                                          \
    ptr += n;                                                                  \
    break;

#  define CHECK_NAME_CASES(enc, ptr, end, nextTokPtr)                          \
  case BT_NONASCII:                                                            \
    if (! IS_NAME_CHAR_MINBPC(enc, ptr)) {                                     \
      *nextTokPtr = ptr;                                                       \
      return XML_TOK_INVALID;                                                  \
    }                                                                          \
    /* fall through */                                                         \
  case BT_NMSTRT:                                                              \
  case BT_HEX:                                                                 \
  case BT_DIGIT:                                                               \
  case BT_NAME:                                                                \
  case BT_MINUS:                                                               \
    ptr += MINBPC(enc);                                                        \
    break;                                                                     \
    CHECK_NAME_CASE(2, enc, ptr, end, nextTokPtr)                              \
    CHECK_NAME_CASE(3, enc, ptr, end, nextTokPtr)                              \
    CHECK_NAME_CASE(4, enc, ptr, end, nextTokPtr)

#  define CHECK_NMSTRT_CASE(n, enc, ptr, end, nextTokPtr)                      \
  case BT_LEAD##n:                                                             \
    if (end - ptr < n)                                                         \
      return XML_TOK_PARTIAL_CHAR;                                             \
    if (! IS_NMSTRT_CHAR(enc, ptr, n)) {                                       \
      *nextTokPtr = ptr;                                                       \
      return XML_TOK_INVALID;                                                  \
    }                                                                          \
    ptr += n;                                                                  \
    break;

#  define CHECK_NMSTRT_CASES(enc, ptr, end, nextTokPtr)                        \
  case BT_NONASCII:                                                            \
    if (! IS_NMSTRT_CHAR_MINBPC(enc, ptr)) {                                   \
      *nextTokPtr = ptr;                                                       \
      return XML_TOK_INVALID;                                                  \
    }                                                                          \
    /* fall through */                                                         \
  case BT_NMSTRT:                                                              \
  case BT_HEX:                                                                 \
    ptr += MINBPC(enc);                                                        \
    break;                                                                     \
    CHECK_NMSTRT_CASE(2, enc, ptr, end, nextTokPtr)                            \
    CHECK_NMSTRT_CASE(3, enc, ptr, end, nextTokPtr)                            \
    CHECK_NMSTRT_CASE(4, enc, ptr, end, nextTokPtr)

#  ifndef PREFIX
#    define PREFIX(ident) ident
#  endif

#  define HAS_CHARS(enc, ptr, end, count) (end - ptr >= count * MINBPC(enc))

#  define HAS_CHAR(enc, ptr, end) HAS_CHARS(enc, ptr, end, 1)

#  define REQUIRE_CHARS(enc, ptr, end, count)                                  \
    {                                                                          \
      if (! HAS_CHARS(enc, ptr, end, count)) {                                 \
        return XML_TOK_PARTIAL;                                                \
      }                                                                        \
    }

#  define REQUIRE_CHAR(enc, ptr, end) REQUIRE_CHARS(enc, ptr, end, 1)

/* ptr points to character following "<!-" */

static int PTRCALL
PREFIX(scanComment)(const ENCODING *enc, const char *ptr, const char *end,
                    const char **nextTokPtr) {
  if (HAS_CHAR(enc, ptr, end)) {
    if (! CHAR_MATCHES(enc, ptr, ASCII_MINUS)) {
      *nextTokPtr = ptr;
      return XML_TOK_INVALID;
    }
    ptr += MINBPC(enc);
    while (HAS_CHAR(enc, ptr, end)) {
      switch (BYTE_TYPE(enc, ptr)) {
        INVALID_CASES(ptr, nextTokPtr)
      case BT_MINUS:
        ptr += MINBPC(enc);
        REQUIRE_CHAR(enc, ptr, end);
        if (CHAR_MATCHES(enc, ptr, ASCII_MINUS)) {
          ptr += MINBPC(enc);
          REQUIRE_CHAR(enc, ptr, end);
          if (! CHAR_MATCHES(enc, ptr, ASCII_GT)) {
            *nextTokPtr = ptr;
            return XML_TOK_INVALID;
          }
          *nextTokPtr = ptr + MINBPC(enc);
          return XML_TOK_COMMENT;
        }
        break;
      default:
        ptr += MINBPC(enc);
        break;
      }
    }
  }
/* This file is included!
                            __  __            _
                         ___\ \/ /_ __   __ _| |_
                        / _ \\  /| '_ \ / _` | __|
                       |  __//  \| |_) | (_| | |_
                        \___/_/\_\ .__/ \__,_|\__|
                                 |_| XML parser

   Copyright (c) 1997-2000 Thai Open Source Software Center Ltd
   Copyright (c) 2000-2017 Expat development team
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

#ifdef XML_TOK_IMPL_C

#  ifndef IS_INVALID_CHAR
#    define IS_INVALID_CHAR(enc, ptr, n) (0)
#  endif

#  define INVALID_LEAD_CASE(n, ptr, nextTokPtr)                                \
  case BT_LEAD##n:                                                             \
    if (end - ptr < n)                                                         \
      return XML_TOK_PARTIAL_CHAR;                                             \
    if (IS_INVALID_CHAR(enc, ptr, n)) {                                        \
      *(nextTokPtr) = (ptr);                                                   \
      return XML_TOK_INVALID;                                                  \
    }                                                                          \
    ptr += n;                                                                  \
    break;

#  define INVALID_CASES(ptr, nextTokPtr)                                       \
    INVALID_LEAD_CASE(2, ptr, nextTokPtr)                                      \
    INVALID_LEAD_CASE(3, ptr, nextTokPtr)                                      \
    INVALID_LEAD_CASE(4, ptr, nextTokPtr)                                      \
  case BT_NONXML:                                                              \
  case BT_MALFORM:                                                             \
  case BT_TRAIL:                                                               \
    *(nextTokPtr) = (ptr);                                                     \
    return XML_TOK_INVALID;

#  define CHECK_NAME_CASE(n, enc, ptr, end, nextTokPtr)                        \
  case BT_LEAD##n:                                                             \
    if (end - ptr < n)                                                         \
      return XML_TOK_PARTIAL_CHAR;                                             \
    if (! IS_NAME_CHAR(enc, ptr, n)) {                                         \
      *nextTokPtr = ptr;                                                       \
      return XML_TOK_INVALID;                                                  \
    }                                                                          \
    ptr += n;                                                                  \
    break;

#  define CHECK_NAME_CASES(enc, ptr, end, nextTokPtr)                          \
  case BT_NONASCII:                                                            \
    if (! IS_NAME_CHAR_MINBPC(enc, ptr)) {                                     \
      *nextTokPtr = ptr;                                                       \
      return XML_TOK_INVALID;                                                  \
    }                                                                          \
    /* fall through */                                                         \
  case BT_NMSTRT:                                                              \
  case BT_HEX:                                                                 \
  case BT_DIGIT:                                                               \
  case BT_NAME:                                                                \
  case BT_MINUS:                                                               \
    ptr += MINBPC(enc);                                                        \
    break;                                                                     \
    CHECK_NAME_CASE(2, enc, ptr, end, nextTokPtr)                              \
    CHECK_NAME_CASE(3, enc, ptr, end, nextTokPtr)                              \
    CHECK_NAME_CASE(4, enc, ptr, end, nextTokPtr)

#  define CHECK_NMSTRT_CASE(n, enc, ptr, end, nextTokPtr)                      \
  case BT_LEAD##n:                                                             \
    if (end - ptr < n)                                                         \
      return XML_TOK_PARTIAL_CHAR;                                             \
    if (! IS_NMSTRT_CHAR(enc, ptr, n)) {                                       \
      *nextTokPtr = ptr;                                                       \
      return XML_TOK_INVALID;                                                  \
    }                                                                          \
    ptr += n;                                                                  \
    break;

#  define CHECK_NMSTRT_CASES(enc, ptr, end, nextTokPtr)                        \
  case BT_NONASCII:                                                            \
    if (! IS_NMSTRT_CHAR_MINBPC(enc, ptr)) {                                   \
      *nextTokPtr = ptr;                                                       \
      return XML_TOK_INVALID;                                                  \
    }                                                                          \
    /* fall through */                                                         \
  case BT_NMSTRT:                                                              \
  case BT_HEX:                                                                 \
    ptr += MINBPC(enc);                                                        \
    break;                                                                     \
    CHECK_NMSTRT_CASE(2, enc, ptr, end, nextTokPtr)                            \
    CHECK_NMSTRT_CASE(3, enc, ptr, end, nextTokPtr)                            \
    CHECK_NMSTRT_CASE(4, enc, ptr, end, nextTokPtr)

#  ifndef PREFIX
#    define PREFIX(ident) ident
#  endif

#  define HAS_CHARS(enc, ptr, end, count) (end - ptr >= count * MINBPC(enc))

#  define HAS_CHAR(enc, ptr, end) HAS_CHARS(enc, ptr, end, 1)

#  define REQUIRE_CHARS(enc, ptr, end, count)                                  \
    {                                                                          \
      if (! HAS_CHARS(enc, ptr, end, count)) {                                 \
        return XML_TOK_PARTIAL;                                                \
      }                                                                        \
    }

#  define REQUIRE_CHAR(enc, ptr, end) REQUIRE_CHARS(enc, ptr, end, 1)

/* ptr points to character following "<!-" */

static int PTRCALL
PREFIX(scanComment)(const ENCODING *enc, const char *ptr, const char *end,
                    const char **nextTokPtr) {
  if (HAS_CHAR(enc, ptr, end)) {
    if (! CHAR_MATCHES(enc, ptr, ASCII_MINUS)) {
      *nextTokPtr = ptr;
      return XML_TOK_INVALID;
    }
    ptr += MINBPC(enc);
    while (HAS_CHAR(enc, ptr, end)) {
      switch (BYTE_TYPE(enc, ptr)) {
        INVALID_CASES(ptr, nextTokPtr)
      case BT_MINUS:
        ptr += MINBPC(enc);
        REQUIRE_CHAR(enc, ptr, end);
        if (CHAR_MATCHES(enc, ptr, ASCII_MINUS)) {
          ptr += MINBPC(enc);
          REQUIRE_CHAR(enc, ptr, end);
          if (! CHAR_MATCHES(enc, ptr, ASCII_GT)) {
            *nextTokPtr = ptr;
            return XML_TOK_INVALID;
          }
          *nextTokPtr = ptr + MINBPC(enc);
          return XML_TOK_COMMENT;
        }
        break;
      default:
        ptr += MINBPC(enc);
        break;
      }
    }
  }
/* This file is included!
                            __  __            _
                         ___\ \/ /_ __   __ _| |_
                        / _ \\  /| '_ \ / _` | __|
                       |  __//  \| |_) | (_| | |_
                        \___/_/\_\ .__/ \__,_|\__|
                                 |_| XML parser

   Copyright (c) 1997-2000 Thai Open Source Software Center Ltd
   Copyright (c) 2000-2017 Expat development team
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

#ifdef XML_TOK_IMPL_C

#  ifndef IS_INVALID_CHAR
#    define IS_INVALID_CHAR(enc, ptr, n) (0)
#  endif

#  define INVALID_LEAD_CASE(n, ptr, nextTokPtr)                                \
  case BT_LEAD##n:                                                             \
    if (end - ptr < n)                                                         \
      return XML_TOK_PARTIAL_CHAR;                                             \
    if (IS_INVALID_CHAR(enc, ptr, n)) {                                        \
      *(nextTokPtr) = (ptr);                                                   \
      return XML_TOK_INVALID;                                                  \
    }                                                                          \
    ptr += n;                                                                  \
    break;

#  define INVALID_CASES(ptr, nextTokPtr)                                       \
    INVALID_LEAD_CASE(2, ptr, nextTokPtr)                                      \
    INVALID_LEAD_CASE(3, ptr, nextTokPtr)                                      \
    INVALID_LEAD_CASE(4, ptr, nextTokPtr)                                      \
  case BT_NONXML:                                                              \
  case BT_MALFORM:                                                             \
  case BT_TRAIL:                                                               \
    *(nextTokPtr) = (ptr);                                                     \
    return XML_TOK_INVALID;

#  define CHECK_NAME_CASE(n, enc, ptr, end, nextTokPtr)                        \
  case BT_LEAD##n:                                                             \
    if (end - ptr < n)                                                         \
      return XML_TOK_PARTIAL_CHAR;                                             \
    if (! IS_NAME_CHAR(enc, ptr, n)) {                                         \
      *nextTokPtr = ptr;                                                       \
      return XML_TOK_INVALID;                                                  \
    }                                                                          \
    ptr += n;                                                                  \
    break;

#  define CHECK_NAME_CASES(enc, ptr, end, nextTokPtr)                          \
  case BT_NONASCII:                                                            \
    if (! IS_NAME_CHAR_MINBPC(enc, ptr)) {                                     \
      *nextTokPtr = ptr;                                                       \
      return XML_TOK_INVALID;                                                  \
    }                                                                          \
    /* fall through */                                                         \
  case BT_NMSTRT:                                                              \
  case BT_HEX:                                                                 \
  case BT_DIGIT:                                                               \
  case BT_NAME:                                                                \
  case BT_MINUS:                                                               \
    ptr += MINBPC(enc);                                                        \
    break;                                                                     \
    CHECK_NAME_CASE(2, enc, ptr, end, nextTokPtr)                              \
    CHECK_NAME_CASE(3, enc, ptr, end, nextTokPtr)                              \
    CHECK_NAME_CASE(4, enc, ptr, end, nextTokPtr)

#  define CHECK_NMSTRT_CASE(n, enc, ptr, end, nextTokPtr)                      \
  case BT_LEAD##n:                                                             \
    if (end - ptr < n)                                                         \
      return XML_TOK_PARTIAL_CHAR;                                             \
    if (! IS_NMSTRT_CHAR(enc, ptr, n)) {                                       \
      *nextTokPtr = ptr;                                                       \
      return XML_TOK_INVALID;                                                  \
    }                                                                          \
    ptr += n;                                                                  \
    break;

#  define CHECK_NMSTRT_CASES(enc, ptr, end, nextTokPtr)                        \
  case BT_NONASCII:                                                            \
    if (! IS_NMSTRT_CHAR_MINBPC(enc, ptr)) {                                   \
      *nextTokPtr = ptr;                                                       \
      return XML_TOK_INVALID;                                                  \
    }                                                                          \
    /* fall through */                                                         \
  case BT_NMSTRT:                                                              \
  case BT_HEX:                                                                 \
    ptr += MINBPC(enc);                                                        \
    break;                                                                     \
    CHECK_NMSTRT_CASE(2, enc, ptr, end, nextTokPtr)                            \
    CHECK_NMSTRT_CASE(3, enc, ptr, end, nextTokPtr)                            \
    CHECK_NMSTRT_CASE(4, enc, ptr, end, nextTokPtr)

#  ifndef PREFIX
#    define PREFIX(ident) ident
#  endif

#  define HAS_CHARS(enc, ptr, end, count) (end - ptr >= count * MINBPC(enc))

#  define HAS_CHAR(enc, ptr, end) HAS_CHARS(enc, ptr, end, 1)

#  define REQUIRE_CHARS(enc, ptr, end, count)                                  \
    {                                                                          \
      if (! HAS_CHARS(enc, ptr, end, count)) {                                 \
        return XML_TOK_PARTIAL;                                                \
      }                                                                        \
    }

#  define REQUIRE_CHAR(enc, ptr, end) REQUIRE_CHARS(enc, ptr, end, 1)

/* ptr points to character following "<!-" */

static int PTRCALL
PREFIX(scanComment)(const ENCODING *enc, const char *ptr, const char *end,
                    const char **nextTokPtr) {
  if (HAS_CHAR(enc, ptr, end)) {
    if (! CHAR_MATCHES(enc, ptr, ASCII_MINUS)) {
      *nextTokPtr = ptr;
      return XML_TOK_INVALID;
    }
    ptr += MINBPC(enc);
    while (HAS_CHAR(enc, ptr, end)) {
      switch (BYTE_TYPE(enc, ptr)) {
        INVALID_CASES(ptr, nextTokPtr)
      case BT_MINUS:
        ptr += MINBPC(enc);
        REQUIRE_CHAR(enc, ptr, end);
        if (CHAR_MATCHES(enc, ptr, ASCII_MINUS)) {
          ptr += MINBPC(enc);
          REQUIRE_CHAR(enc, ptr, end);
          if (! CHAR_MATCHES(enc, ptr, ASCII_GT)) {
            *nextTokPtr = ptr;
            return XML_TOK_INVALID;
          }
          *nextTokPtr = ptr + MINBPC(enc);
          return XML_TOK_COMMENT;
        }
        break;
      default:
        ptr += MINBPC(enc);
        break;
      }
    }
  }
    switch (BYTE_TYPE(enc, ptr)) {
#  define LEAD_CASE(n)                                                         \
  case BT_LEAD##n:                                                             \
    ptr += n;                                                                  \
    break;
      LEAD_CASE(2)
      LEAD_CASE(3)
      LEAD_CASE(4)
#  undef LEAD_CASE
    case BT_LF:
      pos->columnNumber = (XML_Size)-1;
      pos->lineNumber++;
      ptr += MINBPC(enc);
      break;
    case BT_CR:
      pos->lineNumber++;
      ptr += MINBPC(enc);
      if (HAS_CHAR(enc, ptr, end) && BYTE_TYPE(enc, ptr) == BT_LF)
        ptr += MINBPC(enc);
      pos->columnNumber = (XML_Size)-1;
      break;
    default:
      ptr += MINBPC(enc);
      break;
    }
    pos->columnNumber++;
  }
}

#  undef DO_LEAD_CASE
#  undef MULTIBYTE_CASES
#  undef INVALID_CASES
#  undef CHECK_NAME_CASE
#  undef CHECK_NAME_CASES
#  undef CHECK_NMSTRT_CASE
#  undef CHECK_NMSTRT_CASES

#endif /* XML_TOK_IMPL_C */
    switch (BYTE_TYPE(enc, ptr)) {
#  define LEAD_CASE(n)                                                         \
  case BT_LEAD##n:                                                             \
    ptr += n;                                                                  \
    break;
      LEAD_CASE(2)
      LEAD_CASE(3)
      LEAD_CASE(4)
#  undef LEAD_CASE
    case BT_LF:
      pos->columnNumber = (XML_Size)-1;
      pos->lineNumber++;
      ptr += MINBPC(enc);
      break;
    case BT_CR:
      pos->lineNumber++;
      ptr += MINBPC(enc);
      if (HAS_CHAR(enc, ptr, end) && BYTE_TYPE(enc, ptr) == BT_LF)
        ptr += MINBPC(enc);
      pos->columnNumber = (XML_Size)-1;
      break;
    default:
      ptr += MINBPC(enc);
      break;
    }
    pos->columnNumber++;
  }
}

#  undef DO_LEAD_CASE
#  undef MULTIBYTE_CASES
#  undef INVALID_CASES
#  undef CHECK_NAME_CASE
#  undef CHECK_NAME_CASES
#  undef CHECK_NMSTRT_CASE
#  undef CHECK_NMSTRT_CASES

#endif /* XML_TOK_IMPL_C */