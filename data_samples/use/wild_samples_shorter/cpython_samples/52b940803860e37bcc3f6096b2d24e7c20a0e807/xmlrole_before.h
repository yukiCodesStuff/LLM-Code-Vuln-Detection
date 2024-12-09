#ifdef __VMS
/*      0        1         2         3      0        1         2         3
        1234567890123456789012345678901     1234567890123456789012345678901 */
#define XmlPrologStateInitExternalEntity    XmlPrologStateInitExternalEnt
#endif

#include "xmltok.h"

};

typedef struct prolog_state {
  int (PTRCALL *handler) (struct prolog_state *state,
                          int tok,
                          const char *ptr,
                          const char *end,
                          const ENCODING *enc);
  unsigned level;
  int role_none;
#ifdef XML_DTD
  unsigned includeLevel;
void XmlPrologStateInitExternalEntity(PROLOG_STATE *);
#endif /* XML_DTD */

#define XmlTokenRole(state, tok, ptr, end, enc) \
 (((state)->handler)(state, tok, ptr, end, enc))

#ifdef __cplusplus
}
#endif