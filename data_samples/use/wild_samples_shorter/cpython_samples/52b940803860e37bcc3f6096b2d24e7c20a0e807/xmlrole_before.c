#include <stddef.h>

#ifdef _WIN32
#include "winconfig.h"
#else
#ifdef HAVE_EXPAT_CONFIG_H
#include <expat_config.h>
#endif
#endif /* ndef _WIN32 */

#include "expat_external.h"
#include "internal.h"

*/

static const char KW_ANY[] = {
    ASCII_A, ASCII_N, ASCII_Y, '\0' };
static const char KW_ATTLIST[] = {
    ASCII_A, ASCII_T, ASCII_T, ASCII_L, ASCII_I, ASCII_S, ASCII_T, '\0' };
static const char KW_CDATA[] = {
    ASCII_C, ASCII_D, ASCII_A, ASCII_T, ASCII_A, '\0' };
static const char KW_DOCTYPE[] = {
    ASCII_D, ASCII_O, ASCII_C, ASCII_T, ASCII_Y, ASCII_P, ASCII_E, '\0' };
static const char KW_ELEMENT[] = {
    ASCII_E, ASCII_L, ASCII_E, ASCII_M, ASCII_E, ASCII_N, ASCII_T, '\0' };
static const char KW_EMPTY[] = {
    ASCII_E, ASCII_M, ASCII_P, ASCII_T, ASCII_Y, '\0' };
static const char KW_ENTITIES[] = {
    ASCII_E, ASCII_N, ASCII_T, ASCII_I, ASCII_T, ASCII_I, ASCII_E, ASCII_S,
    '\0' };
static const char KW_ENTITY[] = {
    ASCII_E, ASCII_N, ASCII_T, ASCII_I, ASCII_T, ASCII_Y, '\0' };
static const char KW_FIXED[] = {
    ASCII_F, ASCII_I, ASCII_X, ASCII_E, ASCII_D, '\0' };
static const char KW_ID[] = {
    ASCII_I, ASCII_D, '\0' };
static const char KW_IDREF[] = {
    ASCII_I, ASCII_D, ASCII_R, ASCII_E, ASCII_F, '\0' };
static const char KW_IDREFS[] = {
    ASCII_I, ASCII_D, ASCII_R, ASCII_E, ASCII_F, ASCII_S, '\0' };
#ifdef XML_DTD
static const char KW_IGNORE[] = {
    ASCII_I, ASCII_G, ASCII_N, ASCII_O, ASCII_R, ASCII_E, '\0' };
#endif
static const char KW_IMPLIED[] = {
    ASCII_I, ASCII_M, ASCII_P, ASCII_L, ASCII_I, ASCII_E, ASCII_D, '\0' };
#ifdef XML_DTD
static const char KW_INCLUDE[] = {
    ASCII_I, ASCII_N, ASCII_C, ASCII_L, ASCII_U, ASCII_D, ASCII_E, '\0' };
#endif
static const char KW_NDATA[] = {
    ASCII_N, ASCII_D, ASCII_A, ASCII_T, ASCII_A, '\0' };
static const char KW_NMTOKEN[] = {
    ASCII_N, ASCII_M, ASCII_T, ASCII_O, ASCII_K, ASCII_E, ASCII_N, '\0' };
static const char KW_NMTOKENS[] = {
    ASCII_N, ASCII_M, ASCII_T, ASCII_O, ASCII_K, ASCII_E, ASCII_N, ASCII_S,
    '\0' };
static const char KW_NOTATION[] =
    { ASCII_N, ASCII_O, ASCII_T, ASCII_A, ASCII_T, ASCII_I, ASCII_O, ASCII_N,
      '\0' };
static const char KW_PCDATA[] = {
    ASCII_P, ASCII_C, ASCII_D, ASCII_A, ASCII_T, ASCII_A, '\0' };
static const char KW_PUBLIC[] = {
    ASCII_P, ASCII_U, ASCII_B, ASCII_L, ASCII_I, ASCII_C, '\0' };
static const char KW_REQUIRED[] = {
    ASCII_R, ASCII_E, ASCII_Q, ASCII_U, ASCII_I, ASCII_R, ASCII_E, ASCII_D,
    '\0' };
static const char KW_SYSTEM[] = {
    ASCII_S, ASCII_Y, ASCII_S, ASCII_T, ASCII_E, ASCII_M, '\0' };

#ifndef MIN_BYTES_PER_CHAR
#define MIN_BYTES_PER_CHAR(enc) ((enc)->minBytesPerChar)
#endif

#ifdef XML_DTD
#define setTopLevel(state) \
  ((state)->handler = ((state)->documentEntity \
                       ? internalSubset \
                       : externalSubset1))
#else /* not XML_DTD */
#define setTopLevel(state) ((state)->handler = internalSubset)
#endif /* not XML_DTD */

typedef int PTRCALL PROLOG_HANDLER(PROLOG_STATE *state,
                                   int tok,
                                   const char *ptr,
                                   const char *end,
                                   const ENCODING *enc);

static PROLOG_HANDLER
  prolog0, prolog1, prolog2,
  doctype0, doctype1, doctype2, doctype3, doctype4, doctype5,
  internalSubset,
  entity0, entity1, entity2, entity3, entity4, entity5, entity6,
  entity7, entity8, entity9, entity10,
  notation0, notation1, notation2, notation3, notation4,
  attlist0, attlist1, attlist2, attlist3, attlist4, attlist5, attlist6,
  attlist7, attlist8, attlist9,
  element0, element1, element2, element3, element4, element5, element6,
  element7,
#ifdef XML_DTD
  externalSubset0, externalSubset1,
  condSect0, condSect1, condSect2,
#endif /* XML_DTD */
  declClose,
  error;

static int FASTCALL common(PROLOG_STATE *state, int tok);

static int PTRCALL
prolog0(PROLOG_STATE *state,
        int tok,
        const char *ptr,
        const char *end,
        const ENCODING *enc)
{
  switch (tok) {
  case XML_TOK_PROLOG_S:
    state->handler = prolog1;
    return XML_ROLE_NONE;
  case XML_TOK_BOM:
    return XML_ROLE_NONE;
  case XML_TOK_DECL_OPEN:
    if (!XmlNameMatchesAscii(enc,
                             ptr + 2 * MIN_BYTES_PER_CHAR(enc),
                             end,
                             KW_DOCTYPE))
      break;
    state->handler = doctype0;
    return XML_ROLE_DOCTYPE_NONE;
  case XML_TOK_INSTANCE_START:
}

static int PTRCALL
prolog1(PROLOG_STATE *state,
        int tok,
        const char *ptr,
        const char *end,
        const ENCODING *enc)
{
  switch (tok) {
  case XML_TOK_PROLOG_S:
    return XML_ROLE_NONE;
  case XML_TOK_PI:
     */
    return XML_ROLE_NONE; /* LCOV_EXCL_LINE */
  case XML_TOK_DECL_OPEN:
    if (!XmlNameMatchesAscii(enc,
                             ptr + 2 * MIN_BYTES_PER_CHAR(enc),
                             end,
                             KW_DOCTYPE))
      break;
    state->handler = doctype0;
    return XML_ROLE_DOCTYPE_NONE;
  case XML_TOK_INSTANCE_START:
}

static int PTRCALL
prolog2(PROLOG_STATE *state,
        int tok,
        const char *UNUSED_P(ptr),
        const char *UNUSED_P(end),
        const ENCODING *UNUSED_P(enc))
{
  switch (tok) {
  case XML_TOK_PROLOG_S:
    return XML_ROLE_NONE;
  case XML_TOK_PI:
}

static int PTRCALL
doctype0(PROLOG_STATE *state,
         int tok,
         const char *UNUSED_P(ptr),
         const char *UNUSED_P(end),
         const ENCODING *UNUSED_P(enc))
{
  switch (tok) {
  case XML_TOK_PROLOG_S:
    return XML_ROLE_DOCTYPE_NONE;
  case XML_TOK_NAME:
}

static int PTRCALL
doctype1(PROLOG_STATE *state,
         int tok,
         const char *ptr,
         const char *end,
         const ENCODING *enc)
{
  switch (tok) {
  case XML_TOK_PROLOG_S:
    return XML_ROLE_DOCTYPE_NONE;
  case XML_TOK_OPEN_BRACKET:
}

static int PTRCALL
doctype2(PROLOG_STATE *state,
         int tok,
         const char *UNUSED_P(ptr),
         const char *UNUSED_P(end),
         const ENCODING *UNUSED_P(enc))
{
  switch (tok) {
  case XML_TOK_PROLOG_S:
    return XML_ROLE_DOCTYPE_NONE;
  case XML_TOK_LITERAL:
}

static int PTRCALL
doctype3(PROLOG_STATE *state,
         int tok,
         const char *UNUSED_P(ptr),
         const char *UNUSED_P(end),
         const ENCODING *UNUSED_P(enc))
{
  switch (tok) {
  case XML_TOK_PROLOG_S:
    return XML_ROLE_DOCTYPE_NONE;
  case XML_TOK_LITERAL:
}

static int PTRCALL
doctype4(PROLOG_STATE *state,
         int tok,
         const char *UNUSED_P(ptr),
         const char *UNUSED_P(end),
         const ENCODING *UNUSED_P(enc))
{
  switch (tok) {
  case XML_TOK_PROLOG_S:
    return XML_ROLE_DOCTYPE_NONE;
  case XML_TOK_OPEN_BRACKET:
}

static int PTRCALL
doctype5(PROLOG_STATE *state,
         int tok,
         const char *UNUSED_P(ptr),
         const char *UNUSED_P(end),
         const ENCODING *UNUSED_P(enc))
{
  switch (tok) {
  case XML_TOK_PROLOG_S:
    return XML_ROLE_DOCTYPE_NONE;
  case XML_TOK_DECL_CLOSE:
}

static int PTRCALL
internalSubset(PROLOG_STATE *state,
               int tok,
               const char *ptr,
               const char *end,
               const ENCODING *enc)
{
  switch (tok) {
  case XML_TOK_PROLOG_S:
    return XML_ROLE_NONE;
  case XML_TOK_DECL_OPEN:
    if (XmlNameMatchesAscii(enc,
                            ptr + 2 * MIN_BYTES_PER_CHAR(enc),
                            end,
                            KW_ENTITY)) {
      state->handler = entity0;
      return XML_ROLE_ENTITY_NONE;
    }
    if (XmlNameMatchesAscii(enc,
                            ptr + 2 * MIN_BYTES_PER_CHAR(enc),
                            end,
                            KW_ATTLIST)) {
      state->handler = attlist0;
      return XML_ROLE_ATTLIST_NONE;
    }
    if (XmlNameMatchesAscii(enc,
                            ptr + 2 * MIN_BYTES_PER_CHAR(enc),
                            end,
                            KW_ELEMENT)) {
      state->handler = element0;
      return XML_ROLE_ELEMENT_NONE;
    }
    if (XmlNameMatchesAscii(enc,
                            ptr + 2 * MIN_BYTES_PER_CHAR(enc),
                            end,
                            KW_NOTATION)) {
      state->handler = notation0;
      return XML_ROLE_NOTATION_NONE;
    }
#ifdef XML_DTD

static int PTRCALL
externalSubset0(PROLOG_STATE *state,
                int tok,
                const char *ptr,
                const char *end,
                const ENCODING *enc)
{
  state->handler = externalSubset1;
  if (tok == XML_TOK_XML_DECL)
    return XML_ROLE_TEXT_DECL;
  return externalSubset1(state, tok, ptr, end, enc);
}

static int PTRCALL
externalSubset1(PROLOG_STATE *state,
                int tok,
                const char *ptr,
                const char *end,
                const ENCODING *enc)
{
  switch (tok) {
  case XML_TOK_COND_SECT_OPEN:
    state->handler = condSect0;
    return XML_ROLE_NONE;
#endif /* XML_DTD */

static int PTRCALL
entity0(PROLOG_STATE *state,
        int tok,
        const char *UNUSED_P(ptr),
        const char *UNUSED_P(end),
        const ENCODING *UNUSED_P(enc))
{
  switch (tok) {
  case XML_TOK_PROLOG_S:
    return XML_ROLE_ENTITY_NONE;
  case XML_TOK_PERCENT:
}

static int PTRCALL
entity1(PROLOG_STATE *state,
        int tok,
        const char *UNUSED_P(ptr),
        const char *UNUSED_P(end),
        const ENCODING *UNUSED_P(enc))
{
  switch (tok) {
  case XML_TOK_PROLOG_S:
    return XML_ROLE_ENTITY_NONE;
  case XML_TOK_NAME:
}

static int PTRCALL
entity2(PROLOG_STATE *state,
        int tok,
        const char *ptr,
        const char *end,
        const ENCODING *enc)
{
  switch (tok) {
  case XML_TOK_PROLOG_S:
    return XML_ROLE_ENTITY_NONE;
  case XML_TOK_NAME:
}

static int PTRCALL
entity3(PROLOG_STATE *state,
        int tok,
        const char *UNUSED_P(ptr),
        const char *UNUSED_P(end),
        const ENCODING *UNUSED_P(enc))
{
  switch (tok) {
  case XML_TOK_PROLOG_S:
    return XML_ROLE_ENTITY_NONE;
  case XML_TOK_LITERAL:
}

static int PTRCALL
entity4(PROLOG_STATE *state,
        int tok,
        const char *UNUSED_P(ptr),
        const char *UNUSED_P(end),
        const ENCODING *UNUSED_P(enc))
{
  switch (tok) {
  case XML_TOK_PROLOG_S:
    return XML_ROLE_ENTITY_NONE;
  case XML_TOK_LITERAL:
}

static int PTRCALL
entity5(PROLOG_STATE *state,
        int tok,
        const char *ptr,
        const char *end,
        const ENCODING *enc)
{
  switch (tok) {
  case XML_TOK_PROLOG_S:
    return XML_ROLE_ENTITY_NONE;
  case XML_TOK_DECL_CLOSE:
}

static int PTRCALL
entity6(PROLOG_STATE *state,
        int tok,
        const char *UNUSED_P(ptr),
        const char *UNUSED_P(end),
        const ENCODING *UNUSED_P(enc))
{
  switch (tok) {
  case XML_TOK_PROLOG_S:
    return XML_ROLE_ENTITY_NONE;
  case XML_TOK_NAME:
}

static int PTRCALL
entity7(PROLOG_STATE *state,
        int tok,
        const char *ptr,
        const char *end,
        const ENCODING *enc)
{
  switch (tok) {
  case XML_TOK_PROLOG_S:
    return XML_ROLE_ENTITY_NONE;
  case XML_TOK_NAME:
}

static int PTRCALL
entity8(PROLOG_STATE *state,
        int tok,
        const char *UNUSED_P(ptr),
        const char *UNUSED_P(end),
        const ENCODING *UNUSED_P(enc))
{
  switch (tok) {
  case XML_TOK_PROLOG_S:
    return XML_ROLE_ENTITY_NONE;
  case XML_TOK_LITERAL:
}

static int PTRCALL
entity9(PROLOG_STATE *state,
        int tok,
        const char *UNUSED_P(ptr),
        const char *UNUSED_P(end),
        const ENCODING *UNUSED_P(enc))
{
  switch (tok) {
  case XML_TOK_PROLOG_S:
    return XML_ROLE_ENTITY_NONE;
  case XML_TOK_LITERAL:
}

static int PTRCALL
entity10(PROLOG_STATE *state,
         int tok,
         const char *UNUSED_P(ptr),
         const char *UNUSED_P(end),
         const ENCODING *UNUSED_P(enc))
{
  switch (tok) {
  case XML_TOK_PROLOG_S:
    return XML_ROLE_ENTITY_NONE;
  case XML_TOK_DECL_CLOSE:
}

static int PTRCALL
notation0(PROLOG_STATE *state,
          int tok,
          const char *UNUSED_P(ptr),
          const char *UNUSED_P(end),
          const ENCODING *UNUSED_P(enc))
{
  switch (tok) {
  case XML_TOK_PROLOG_S:
    return XML_ROLE_NOTATION_NONE;
  case XML_TOK_NAME:
}

static int PTRCALL
notation1(PROLOG_STATE *state,
          int tok,
          const char *ptr,
          const char *end,
          const ENCODING *enc)
{
  switch (tok) {
  case XML_TOK_PROLOG_S:
    return XML_ROLE_NOTATION_NONE;
  case XML_TOK_NAME:
}

static int PTRCALL
notation2(PROLOG_STATE *state,
          int tok,
          const char *UNUSED_P(ptr),
          const char *UNUSED_P(end),
          const ENCODING *UNUSED_P(enc))
{
  switch (tok) {
  case XML_TOK_PROLOG_S:
    return XML_ROLE_NOTATION_NONE;
  case XML_TOK_LITERAL:
}

static int PTRCALL
notation3(PROLOG_STATE *state,
          int tok,
          const char *UNUSED_P(ptr),
          const char *UNUSED_P(end),
          const ENCODING *UNUSED_P(enc))
{
  switch (tok) {
  case XML_TOK_PROLOG_S:
    return XML_ROLE_NOTATION_NONE;
  case XML_TOK_LITERAL:
}

static int PTRCALL
notation4(PROLOG_STATE *state,
          int tok,
          const char *UNUSED_P(ptr),
          const char *UNUSED_P(end),
          const ENCODING *UNUSED_P(enc))
{
  switch (tok) {
  case XML_TOK_PROLOG_S:
    return XML_ROLE_NOTATION_NONE;
  case XML_TOK_LITERAL:
}

static int PTRCALL
attlist0(PROLOG_STATE *state,
         int tok,
         const char *UNUSED_P(ptr),
         const char *UNUSED_P(end),
         const ENCODING *UNUSED_P(enc))
{
  switch (tok) {
  case XML_TOK_PROLOG_S:
    return XML_ROLE_ATTLIST_NONE;
  case XML_TOK_NAME:
}

static int PTRCALL
attlist1(PROLOG_STATE *state,
         int tok,
         const char *UNUSED_P(ptr),
         const char *UNUSED_P(end),
         const ENCODING *UNUSED_P(enc))
{
  switch (tok) {
  case XML_TOK_PROLOG_S:
    return XML_ROLE_ATTLIST_NONE;
  case XML_TOK_DECL_CLOSE:
}

static int PTRCALL
attlist2(PROLOG_STATE *state,
         int tok,
         const char *ptr,
         const char *end,
         const ENCODING *enc)
{
  switch (tok) {
  case XML_TOK_PROLOG_S:
    return XML_ROLE_ATTLIST_NONE;
  case XML_TOK_NAME:
    {
      static const char * const types[] = {
        KW_CDATA,
        KW_ID,
        KW_IDREF,
        KW_IDREFS,
        KW_ENTITY,
        KW_ENTITIES,
        KW_NMTOKEN,
        KW_NMTOKENS,
      };
      int i;
      for (i = 0; i < (int)(sizeof(types)/sizeof(types[0])); i++)
        if (XmlNameMatchesAscii(enc, ptr, end, types[i])) {
          state->handler = attlist8;
          return XML_ROLE_ATTRIBUTE_TYPE_CDATA + i;
        }
    }
    if (XmlNameMatchesAscii(enc, ptr, end, KW_NOTATION)) {
      state->handler = attlist5;
      return XML_ROLE_ATTLIST_NONE;
    }
}

static int PTRCALL
attlist3(PROLOG_STATE *state,
         int tok,
         const char *UNUSED_P(ptr),
         const char *UNUSED_P(end),
         const ENCODING *UNUSED_P(enc))
{
  switch (tok) {
  case XML_TOK_PROLOG_S:
    return XML_ROLE_ATTLIST_NONE;
  case XML_TOK_NMTOKEN:
}

static int PTRCALL
attlist4(PROLOG_STATE *state,
         int tok,
         const char *UNUSED_P(ptr),
         const char *UNUSED_P(end),
         const ENCODING *UNUSED_P(enc))
{
  switch (tok) {
  case XML_TOK_PROLOG_S:
    return XML_ROLE_ATTLIST_NONE;
  case XML_TOK_CLOSE_PAREN:
}

static int PTRCALL
attlist5(PROLOG_STATE *state,
         int tok,
         const char *UNUSED_P(ptr),
         const char *UNUSED_P(end),
         const ENCODING *UNUSED_P(enc))
{
  switch (tok) {
  case XML_TOK_PROLOG_S:
    return XML_ROLE_ATTLIST_NONE;
  case XML_TOK_OPEN_PAREN:
}

static int PTRCALL
attlist6(PROLOG_STATE *state,
         int tok,
         const char *UNUSED_P(ptr),
         const char *UNUSED_P(end),
         const ENCODING *UNUSED_P(enc))
{
  switch (tok) {
  case XML_TOK_PROLOG_S:
    return XML_ROLE_ATTLIST_NONE;
  case XML_TOK_NAME:
}

static int PTRCALL
attlist7(PROLOG_STATE *state,
         int tok,
         const char *UNUSED_P(ptr),
         const char *UNUSED_P(end),
         const ENCODING *UNUSED_P(enc))
{
  switch (tok) {
  case XML_TOK_PROLOG_S:
    return XML_ROLE_ATTLIST_NONE;
  case XML_TOK_CLOSE_PAREN:

/* default value */
static int PTRCALL
attlist8(PROLOG_STATE *state,
         int tok,
         const char *ptr,
         const char *end,
         const ENCODING *enc)
{
  switch (tok) {
  case XML_TOK_PROLOG_S:
    return XML_ROLE_ATTLIST_NONE;
  case XML_TOK_POUND_NAME:
    if (XmlNameMatchesAscii(enc,
                            ptr + MIN_BYTES_PER_CHAR(enc),
                            end,
                            KW_IMPLIED)) {
      state->handler = attlist1;
      return XML_ROLE_IMPLIED_ATTRIBUTE_VALUE;
    }
    if (XmlNameMatchesAscii(enc,
                            ptr + MIN_BYTES_PER_CHAR(enc),
                            end,
                            KW_REQUIRED)) {
      state->handler = attlist1;
      return XML_ROLE_REQUIRED_ATTRIBUTE_VALUE;
    }
    if (XmlNameMatchesAscii(enc,
                            ptr + MIN_BYTES_PER_CHAR(enc),
                            end,
                            KW_FIXED)) {
      state->handler = attlist9;
      return XML_ROLE_ATTLIST_NONE;
    }
}

static int PTRCALL
attlist9(PROLOG_STATE *state,
         int tok,
         const char *UNUSED_P(ptr),
         const char *UNUSED_P(end),
         const ENCODING *UNUSED_P(enc))
{
  switch (tok) {
  case XML_TOK_PROLOG_S:
    return XML_ROLE_ATTLIST_NONE;
  case XML_TOK_LITERAL:
}

static int PTRCALL
element0(PROLOG_STATE *state,
         int tok,
         const char *UNUSED_P(ptr),
         const char *UNUSED_P(end),
         const ENCODING *UNUSED_P(enc))
{
  switch (tok) {
  case XML_TOK_PROLOG_S:
    return XML_ROLE_ELEMENT_NONE;
  case XML_TOK_NAME:
}

static int PTRCALL
element1(PROLOG_STATE *state,
         int tok,
         const char *ptr,
         const char *end,
         const ENCODING *enc)
{
  switch (tok) {
  case XML_TOK_PROLOG_S:
    return XML_ROLE_ELEMENT_NONE;
  case XML_TOK_NAME:
}

static int PTRCALL
element2(PROLOG_STATE *state,
         int tok,
         const char *ptr,
         const char *end,
         const ENCODING *enc)
{
  switch (tok) {
  case XML_TOK_PROLOG_S:
    return XML_ROLE_ELEMENT_NONE;
  case XML_TOK_POUND_NAME:
    if (XmlNameMatchesAscii(enc,
                            ptr + MIN_BYTES_PER_CHAR(enc),
                            end,
                            KW_PCDATA)) {
      state->handler = element3;
      return XML_ROLE_CONTENT_PCDATA;
    }
}

static int PTRCALL
element3(PROLOG_STATE *state,
         int tok,
         const char *UNUSED_P(ptr),
         const char *UNUSED_P(end),
         const ENCODING *UNUSED_P(enc))
{
  switch (tok) {
  case XML_TOK_PROLOG_S:
    return XML_ROLE_ELEMENT_NONE;
  case XML_TOK_CLOSE_PAREN:
}

static int PTRCALL
element4(PROLOG_STATE *state,
         int tok,
         const char *UNUSED_P(ptr),
         const char *UNUSED_P(end),
         const ENCODING *UNUSED_P(enc))
{
  switch (tok) {
  case XML_TOK_PROLOG_S:
    return XML_ROLE_ELEMENT_NONE;
  case XML_TOK_NAME:
}

static int PTRCALL
element5(PROLOG_STATE *state,
         int tok,
         const char *UNUSED_P(ptr),
         const char *UNUSED_P(end),
         const ENCODING *UNUSED_P(enc))
{
  switch (tok) {
  case XML_TOK_PROLOG_S:
    return XML_ROLE_ELEMENT_NONE;
  case XML_TOK_CLOSE_PAREN_ASTERISK:
}

static int PTRCALL
element6(PROLOG_STATE *state,
         int tok,
         const char *UNUSED_P(ptr),
         const char *UNUSED_P(end),
         const ENCODING *UNUSED_P(enc))
{
  switch (tok) {
  case XML_TOK_PROLOG_S:
    return XML_ROLE_ELEMENT_NONE;
  case XML_TOK_OPEN_PAREN:
}

static int PTRCALL
element7(PROLOG_STATE *state,
         int tok,
         const char *UNUSED_P(ptr),
         const char *UNUSED_P(end),
         const ENCODING *UNUSED_P(enc))
{
  switch (tok) {
  case XML_TOK_PROLOG_S:
    return XML_ROLE_ELEMENT_NONE;
  case XML_TOK_CLOSE_PAREN:
#ifdef XML_DTD

static int PTRCALL
condSect0(PROLOG_STATE *state,
          int tok,
          const char *ptr,
          const char *end,
          const ENCODING *enc)
{
  switch (tok) {
  case XML_TOK_PROLOG_S:
    return XML_ROLE_NONE;
  case XML_TOK_NAME:
}

static int PTRCALL
condSect1(PROLOG_STATE *state,
          int tok,
          const char *UNUSED_P(ptr),
          const char *UNUSED_P(end),
          const ENCODING *UNUSED_P(enc))
{
  switch (tok) {
  case XML_TOK_PROLOG_S:
    return XML_ROLE_NONE;
  case XML_TOK_OPEN_BRACKET:
}

static int PTRCALL
condSect2(PROLOG_STATE *state,
          int tok,
          const char *UNUSED_P(ptr),
          const char *UNUSED_P(end),
          const ENCODING *UNUSED_P(enc))
{
  switch (tok) {
  case XML_TOK_PROLOG_S:
    return XML_ROLE_NONE;
  case XML_TOK_OPEN_BRACKET:
#endif /* XML_DTD */

static int PTRCALL
declClose(PROLOG_STATE *state,
          int tok,
          const char *UNUSED_P(ptr),
          const char *UNUSED_P(end),
          const ENCODING *UNUSED_P(enc))
{
  switch (tok) {
  case XML_TOK_PROLOG_S:
    return state->role_none;
  case XML_TOK_DECL_CLOSE:
 * LCOV_EXCL_START
 */
static int PTRCALL
error(PROLOG_STATE *UNUSED_P(state),
      int UNUSED_P(tok),
      const char *UNUSED_P(ptr),
      const char *UNUSED_P(end),
      const ENCODING *UNUSED_P(enc))
{
  return XML_ROLE_NONE;
}
/* LCOV_EXCL_STOP */

static int FASTCALL
common(PROLOG_STATE *state, int tok)
{
#ifdef XML_DTD
  if (!state->documentEntity && tok == XML_TOK_PARAM_ENTITY_REF)
    return XML_ROLE_INNER_PARAM_ENTITY_REF;
#endif
  state->handler = error;
  return XML_ROLE_ERROR;
}

void
XmlPrologStateInit(PROLOG_STATE *state)
{
  state->handler = prolog0;
#ifdef XML_DTD
  state->documentEntity = 1;
  state->includeLevel = 0;
#ifdef XML_DTD

void
XmlPrologStateInitExternalEntity(PROLOG_STATE *state)
{
  state->handler = externalSubset0;
  state->documentEntity = 0;
  state->includeLevel = 0;
}