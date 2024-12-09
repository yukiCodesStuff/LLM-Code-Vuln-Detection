
#include "regint.h"

#if 0
static void
conv_ext0be32(const UChar* s, const UChar* end, UChar* conv)
{
  while (s < end) {

  return ONIGERR_NOT_SUPPORTED_ENCODING_COMBINATION;
}
#endif

extern int
onig_new_deluxe(regex_t** reg, const UChar* pattern, const UChar* pattern_end,
                OnigCompileInfo* ci, OnigErrorInfo* einfo)
  if (IS_NOT_NULL(einfo)) einfo->par = (UChar* )NULL;

  if (ci->pattern_enc != ci->target_enc) {
    return ONIGERR_NOT_SUPPORTED_ENCODING_COMBINATION;
  }
  else {
    cpat     = (UChar* )pattern;
    cpat_end = (UChar* )pattern_end;