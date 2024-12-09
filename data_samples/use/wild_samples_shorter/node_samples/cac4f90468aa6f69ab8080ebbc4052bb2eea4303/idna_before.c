
#include "uv.h"
#include "idna.h"
#include <string.h>

static unsigned uv__utf8_decode1_slow(const char** p,
                                      const char* pe,
  if (a > 0xF7)
    return -1;

  switch (*p - pe) {
  default:
    if (a > 0xEF) {
      min = 0x10000;
      a = a & 7;
      a = 0;
      break;
    }
    return -1;  /* Invalid continuation byte. */
  }

  if (0x80 != (0xC0 & (b ^ c ^ d)))
unsigned uv__utf8_decode1(const char** p, const char* pe) {
  unsigned a;

  a = (unsigned char) *(*p)++;

  if (a < 128)
    return a;  /* ASCII, common case. */
  return uv__utf8_decode1_slow(p, pe, a);
}

#define foreach_codepoint(c, p, pe) \
  for (; (void) (*p <= pe && (c = uv__utf8_decode1(p, pe))), *p <= pe;)

static int uv__idna_toascii_label(const char* s, const char* se,
                                  char** d, char* de) {
  static const char alphabet[] = "abcdefghijklmnopqrstuvwxyz0123456789";
  const char* ss;
  ss = s;
  todo = 0;

  foreach_codepoint(c, &s, se) {
    if (c < 128)
      h++;
    else if (c == (unsigned) -1)
      return UV_EINVAL;
    else
      todo++;
  }

  if (todo > 0) {
    if (*d < de) *(*d)++ = 'x';
    if (*d < de) *(*d)++ = 'n';
    if (*d < de) *(*d)++ = '-';
    if (*d < de) *(*d)++ = '-';
  }

  x = 0;
  s = ss;
  foreach_codepoint(c, &s, se) {
    if (c > 127)
      continue;

    if (*d < de)
  while (todo > 0) {
    m = -1;
    s = ss;
    foreach_codepoint(c, &s, se)
      if (c >= n)
        if (c < m)
          m = c;

    x = m - n;
    y = h + 1;

    n = m;

    s = ss;
    foreach_codepoint(c, &s, se) {
      if (c < n)
        if (++delta == 0)
          return UV_E2BIG;  /* Overflow. */

  return 0;
}

#undef foreach_codepoint

long uv__idna_toascii(const char* s, const char* se, char* d, char* de) {
  const char* si;
  const char* st;
  unsigned c;

  ds = d;

  for (si = s; si < se; /* empty */) {
    st = si;
    c = uv__utf8_decode1(&si, se);

    if (c != '.')
      if (c != 0x3002)  /* 。 */
        if (c != 0xFF0E)  /* ． */
          if (c != 0xFF61)  /* ｡ */