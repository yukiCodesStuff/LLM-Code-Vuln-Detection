/**********************************************************************
  regext.c -  Oniguruma (regular expression library)
**********************************************************************/
/*-
 * Copyright (c) 2002-2008  K.Kosako  <sndgk393 AT ybb DOT ne DOT jp>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "regint.h"

#if 0
static void
conv_ext0be32(const UChar* s, const UChar* end, UChar* conv)
{
  while (s < end) {
    *conv++ = '\0';
    *conv++ = '\0';
    *conv++ = '\0';
    *conv++ = *s++;
  }
}
    else if (from == ONIG_ENCODING_UTF32_BE) {
      goto swap32;
    }
  }

  return ONIGERR_NOT_SUPPORTED_ENCODING_COMBINATION;
}
#endif

extern int
onig_new_deluxe(regex_t** reg, const UChar* pattern, const UChar* pattern_end,
                OnigCompileInfo* ci, OnigErrorInfo* einfo)
{
  int r;
  UChar *cpat, *cpat_end;

  if (IS_NOT_NULL(einfo)) einfo->par = (UChar* )NULL;

  if (ci->pattern_enc != ci->target_enc) {
    return ONIGERR_NOT_SUPPORTED_ENCODING_COMBINATION;
  }
{
  int r;
  UChar *cpat, *cpat_end;

  if (IS_NOT_NULL(einfo)) einfo->par = (UChar* )NULL;

  if (ci->pattern_enc != ci->target_enc) {
    return ONIGERR_NOT_SUPPORTED_ENCODING_COMBINATION;
  }