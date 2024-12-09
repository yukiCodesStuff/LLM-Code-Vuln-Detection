/*
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

#include <stddef.h>

#ifdef _WIN32
#  include "winconfig.h"
#else
#  ifdef HAVE_EXPAT_CONFIG_H
#    include <expat_config.h>
#  endif
#endif /* ndef _WIN32 */

#include "expat_external.h"
#include "internal.h"
#include "xmlrole.h"
#include "ascii.h"

/* Doesn't check:

 that ,| are not mixed in a model group
 content of literals

*/

static const char KW_ANY[] = {ASCII_A, ASCII_N, ASCII_Y, '\0'};
static const char KW_ATTLIST[]
    = {ASCII_A, ASCII_T, ASCII_T, ASCII_L, ASCII_I, ASCII_S, ASCII_T, '\0'};
static const char KW_CDATA[]
    = {ASCII_C, ASCII_D, ASCII_A, ASCII_T, ASCII_A, '\0'};
static const char KW_DOCTYPE[]
    = {ASCII_D, ASCII_O, ASCII_C, ASCII_T, ASCII_Y, ASCII_P, ASCII_E, '\0'};
static const char KW_ELEMENT[]
    = {ASCII_E, ASCII_L, ASCII_E, ASCII_M, ASCII_E, ASCII_N, ASCII_T, '\0'};
static const char KW_EMPTY[]
    = {ASCII_E, ASCII_M, ASCII_P, ASCII_T, ASCII_Y, '\0'};
static const char KW_ENTITIES[] = {ASCII_E, ASCII_N, ASCII_T, ASCII_I, ASCII_T,
                                   ASCII_I, ASCII_E, ASCII_S, '\0'};
static const char KW_ENTITY[]
    = {ASCII_E, ASCII_N, ASCII_T, ASCII_I, ASCII_T, ASCII_Y, '\0'};
static const char KW_FIXED[]
    = {ASCII_F, ASCII_I, ASCII_X, ASCII_E, ASCII_D, '\0'};
static const char KW_ID[] = {ASCII_I, ASCII_D, '\0'};
static const char KW_IDREF[]
    = {ASCII_I, ASCII_D, ASCII_R, ASCII_E, ASCII_F, '\0'};
static const char KW_IDREFS[]
    = {ASCII_I, ASCII_D, ASCII_R, ASCII_E, ASCII_F, ASCII_S, '\0'};
#ifdef XML_DTD
static const char KW_IGNORE[]
    = {ASCII_I, ASCII_G, ASCII_N, ASCII_O, ASCII_R, ASCII_E, '\0'};
#endif
static const char KW_IMPLIED[]
    = {ASCII_I, ASCII_M, ASCII_P, ASCII_L, ASCII_I, ASCII_E, ASCII_D, '\0'};
#ifdef XML_DTD
static const char KW_INCLUDE[]
    = {ASCII_I, ASCII_N, ASCII_C, ASCII_L, ASCII_U, ASCII_D, ASCII_E, '\0'};
#endif
static const char KW_NDATA[]
    = {ASCII_N, ASCII_D, ASCII_A, ASCII_T, ASCII_A, '\0'};
static const char KW_NMTOKEN[]
    = {ASCII_N, ASCII_M, ASCII_T, ASCII_O, ASCII_K, ASCII_E, ASCII_N, '\0'};
static const char KW_NMTOKENS[] = {ASCII_N, ASCII_M, ASCII_T, ASCII_O, ASCII_K,
                                   ASCII_E, ASCII_N, ASCII_S, '\0'};
static const char KW_NOTATION[] = {ASCII_N, ASCII_O, ASCII_T, ASCII_A, ASCII_T,
                                   ASCII_I, ASCII_O, ASCII_N, '\0'};
static const char KW_PCDATA[]
    = {ASCII_P, ASCII_C, ASCII_D, ASCII_A, ASCII_T, ASCII_A, '\0'};
static const char KW_PUBLIC[]
    = {ASCII_P, ASCII_U, ASCII_B, ASCII_L, ASCII_I, ASCII_C, '\0'};
static const char KW_REQUIRED[] = {ASCII_R, ASCII_E, ASCII_Q, ASCII_U, ASCII_I,
                                   ASCII_R, ASCII_E, ASCII_D, '\0'};
static const char KW_SYSTEM[]
    = {ASCII_S, ASCII_Y, ASCII_S, ASCII_T, ASCII_E, ASCII_M, '\0'};

#ifndef MIN_BYTES_PER_CHAR
#  define MIN_BYTES_PER_CHAR(enc) ((enc)->minBytesPerChar)
#endif

#ifdef XML_DTD
#  define setTopLevel(state)                                                   \
    ((state)->handler                                                          \
     = ((state)->documentEntity ? internalSubset : externalSubset1))
#else /* not XML_DTD */
#  define setTopLevel(state) ((state)->handler = internalSubset)
#endif /* not XML_DTD */

typedef int PTRCALL PROLOG_HANDLER(PROLOG_STATE *state, int tok,
                                   const char *ptr, const char *end,
                                   const ENCODING *enc);

static PROLOG_HANDLER prolog0, prolog1, prolog2, doctype0, doctype1, doctype2,
    doctype3, doctype4, doctype5, internalSubset, entity0, entity1, entity2,
    entity3, entity4, entity5, entity6, entity7, entity8, entity9, entity10,
    notation0, notation1, notation2, notation3, notation4, attlist0, attlist1,
    attlist2, attlist3, attlist4, attlist5, attlist6, attlist7, attlist8,
    attlist9, element0, element1, element2, element3, element4, element5,
    element6, element7,
#ifdef XML_DTD
    externalSubset0, externalSubset1, condSect0, condSect1, condSect2,
#endif /* XML_DTD */
    declClose, error;

static int FASTCALL common(PROLOG_STATE *state, int tok);

static int PTRCALL
prolog0(PROLOG_STATE *state, int tok, const char *ptr, const char *end,
        const ENCODING *enc) {
  switch (tok) {
  case XML_TOK_PROLOG_S:
    state->handler = prolog1;
    return XML_ROLE_NONE;
  case XML_TOK_XML_DECL:
    state->handler = prolog1;
    return XML_ROLE_XML_DECL;
  case XML_TOK_PI:
    state->handler = prolog1;
    return XML_ROLE_PI;
  case XML_TOK_COMMENT:
    state->handler = prolog1;
    return XML_ROLE_COMMENT;
  case XML_TOK_BOM:
    return XML_ROLE_NONE;
  case XML_TOK_DECL_OPEN:
    if (! XmlNameMatchesAscii(enc, ptr + 2 * MIN_BYTES_PER_CHAR(enc), end,
                              KW_DOCTYPE))
      break;
    state->handler = doctype0;
    return XML_ROLE_DOCTYPE_NONE;
  case XML_TOK_INSTANCE_START:
    state->handler = error;
    return XML_ROLE_INSTANCE_START;
  }
  return common(state, tok);
}
/*
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

#include <stddef.h>

#ifdef _WIN32
#  include "winconfig.h"
#else
#  ifdef HAVE_EXPAT_CONFIG_H
#    include <expat_config.h>
#  endif
#endif /* ndef _WIN32 */

#include "expat_external.h"
#include "internal.h"
#include "xmlrole.h"
#include "ascii.h"

/* Doesn't check:

 that ,| are not mixed in a model group
 content of literals

*/

static const char KW_ANY[] = {ASCII_A, ASCII_N, ASCII_Y, '\0'};
static const char KW_ATTLIST[]
    = {ASCII_A, ASCII_T, ASCII_T, ASCII_L, ASCII_I, ASCII_S, ASCII_T, '\0'};
static const char KW_CDATA[]
    = {ASCII_C, ASCII_D, ASCII_A, ASCII_T, ASCII_A, '\0'};
static const char KW_DOCTYPE[]
    = {ASCII_D, ASCII_O, ASCII_C, ASCII_T, ASCII_Y, ASCII_P, ASCII_E, '\0'};
static const char KW_ELEMENT[]
    = {ASCII_E, ASCII_L, ASCII_E, ASCII_M, ASCII_E, ASCII_N, ASCII_T, '\0'};
static const char KW_EMPTY[]
    = {ASCII_E, ASCII_M, ASCII_P, ASCII_T, ASCII_Y, '\0'};
static const char KW_ENTITIES[] = {ASCII_E, ASCII_N, ASCII_T, ASCII_I, ASCII_T,
                                   ASCII_I, ASCII_E, ASCII_S, '\0'};
static const char KW_ENTITY[]
    = {ASCII_E, ASCII_N, ASCII_T, ASCII_I, ASCII_T, ASCII_Y, '\0'};
static const char KW_FIXED[]
    = {ASCII_F, ASCII_I, ASCII_X, ASCII_E, ASCII_D, '\0'};
static const char KW_ID[] = {ASCII_I, ASCII_D, '\0'};
static const char KW_IDREF[]
    = {ASCII_I, ASCII_D, ASCII_R, ASCII_E, ASCII_F, '\0'};
static const char KW_IDREFS[]
    = {ASCII_I, ASCII_D, ASCII_R, ASCII_E, ASCII_F, ASCII_S, '\0'};
#ifdef XML_DTD
static const char KW_IGNORE[]
    = {ASCII_I, ASCII_G, ASCII_N, ASCII_O, ASCII_R, ASCII_E, '\0'};
#endif
static const char KW_IMPLIED[]
    = {ASCII_I, ASCII_M, ASCII_P, ASCII_L, ASCII_I, ASCII_E, ASCII_D, '\0'};
#ifdef XML_DTD
static const char KW_INCLUDE[]
    = {ASCII_I, ASCII_N, ASCII_C, ASCII_L, ASCII_U, ASCII_D, ASCII_E, '\0'};
#endif
static const char KW_NDATA[]
    = {ASCII_N, ASCII_D, ASCII_A, ASCII_T, ASCII_A, '\0'};
static const char KW_NMTOKEN[]
    = {ASCII_N, ASCII_M, ASCII_T, ASCII_O, ASCII_K, ASCII_E, ASCII_N, '\0'};
static const char KW_NMTOKENS[] = {ASCII_N, ASCII_M, ASCII_T, ASCII_O, ASCII_K,
                                   ASCII_E, ASCII_N, ASCII_S, '\0'};
static const char KW_NOTATION[] = {ASCII_N, ASCII_O, ASCII_T, ASCII_A, ASCII_T,
                                   ASCII_I, ASCII_O, ASCII_N, '\0'};
static const char KW_PCDATA[]
    = {ASCII_P, ASCII_C, ASCII_D, ASCII_A, ASCII_T, ASCII_A, '\0'};
static const char KW_PUBLIC[]
    = {ASCII_P, ASCII_U, ASCII_B, ASCII_L, ASCII_I, ASCII_C, '\0'};
static const char KW_REQUIRED[] = {ASCII_R, ASCII_E, ASCII_Q, ASCII_U, ASCII_I,
                                   ASCII_R, ASCII_E, ASCII_D, '\0'};
static const char KW_SYSTEM[]
    = {ASCII_S, ASCII_Y, ASCII_S, ASCII_T, ASCII_E, ASCII_M, '\0'};

#ifndef MIN_BYTES_PER_CHAR
#  define MIN_BYTES_PER_CHAR(enc) ((enc)->minBytesPerChar)
#endif

#ifdef XML_DTD
#  define setTopLevel(state)                                                   \
    ((state)->handler                                                          \
     = ((state)->documentEntity ? internalSubset : externalSubset1))
#else /* not XML_DTD */
#  define setTopLevel(state) ((state)->handler = internalSubset)
#endif /* not XML_DTD */

typedef int PTRCALL PROLOG_HANDLER(PROLOG_STATE *state, int tok,
                                   const char *ptr, const char *end,
                                   const ENCODING *enc);

static PROLOG_HANDLER prolog0, prolog1, prolog2, doctype0, doctype1, doctype2,
    doctype3, doctype4, doctype5, internalSubset, entity0, entity1, entity2,
    entity3, entity4, entity5, entity6, entity7, entity8, entity9, entity10,
    notation0, notation1, notation2, notation3, notation4, attlist0, attlist1,
    attlist2, attlist3, attlist4, attlist5, attlist6, attlist7, attlist8,
    attlist9, element0, element1, element2, element3, element4, element5,
    element6, element7,
#ifdef XML_DTD
    externalSubset0, externalSubset1, condSect0, condSect1, condSect2,
#endif /* XML_DTD */
    declClose, error;

static int FASTCALL common(PROLOG_STATE *state, int tok);

static int PTRCALL
prolog0(PROLOG_STATE *state, int tok, const char *ptr, const char *end,
        const ENCODING *enc) {
  switch (tok) {
  case XML_TOK_PROLOG_S:
    state->handler = prolog1;
    return XML_ROLE_NONE;
  case XML_TOK_XML_DECL:
    state->handler = prolog1;
    return XML_ROLE_XML_DECL;
  case XML_TOK_PI:
    state->handler = prolog1;
    return XML_ROLE_PI;
  case XML_TOK_COMMENT:
    state->handler = prolog1;
    return XML_ROLE_COMMENT;
  case XML_TOK_BOM:
    return XML_ROLE_NONE;
  case XML_TOK_DECL_OPEN:
    if (! XmlNameMatchesAscii(enc, ptr + 2 * MIN_BYTES_PER_CHAR(enc), end,
                              KW_DOCTYPE))
      break;
    state->handler = doctype0;
    return XML_ROLE_DOCTYPE_NONE;
  case XML_TOK_INSTANCE_START:
    state->handler = error;
    return XML_ROLE_INSTANCE_START;
  }
  return common(state, tok);
}
common(PROLOG_STATE *state, int tok) {
#ifdef XML_DTD
  if (! state->documentEntity && tok == XML_TOK_PARAM_ENTITY_REF)
    return XML_ROLE_INNER_PARAM_ENTITY_REF;
#endif
  state->handler = error;
  return XML_ROLE_ERROR;
}

void
XmlPrologStateInit(PROLOG_STATE *state) {
  state->handler = prolog0;
#ifdef XML_DTD
  state->documentEntity = 1;
  state->includeLevel = 0;
  state->inEntityValue = 0;
#endif /* XML_DTD */
}

#ifdef XML_DTD

void
XmlPrologStateInitExternalEntity(PROLOG_STATE *state) {
  state->handler = externalSubset0;
  state->documentEntity = 0;
  state->includeLevel = 0;
}

#endif /* XML_DTD */