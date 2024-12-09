
#include "uv.h"
#include "idna.h"
#include <assert.h>
#include <string.h>

static unsigned uv__utf8_decode1_slow(const char** p,
                                      const char* pe,
  if (a > 0xF7)
    return -1;

  switch (pe - *p) {
  default:
    if (a > 0xEF) {
      min = 0x10000;
      a = a & 7;
      a = 0;
      break;
    }
    /* Fall through. */
  case 0:
    return -1;  /* Invalid continuation byte. */
  }

  if (0x80 != (0xC0 & (b ^ c ^ d)))
unsigned uv__utf8_decode1(const char** p, const char* pe) {
  unsigned a;

  assert(*p < pe);

  a = (unsigned char) *(*p)++;

  if (a < 128)
    return a;  /* ASCII, common case. */
  return uv__utf8_decode1_slow(p, pe, a);
}

static int uv__idna_toascii_label(const char* s, const char* se,
                                  char** d, char* de) {
  static const char alphabet[] = "abcdefghijklmnopqrstuvwxyz0123456789";
  const char* ss;
  ss = s;
  todo = 0;

  /* Note: after this loop we've visited all UTF-8 characters and know
   * they're legal so we no longer need to check for decode errors.
   */
  while (s < se) {
    c = uv__utf8_decode1(&s, se);

    if (c == -1u)
      return UV_EINVAL;

    if (c < 128)
      h++;
    else
      todo++;
  }

  /* Only write "xn--" when there are non-ASCII characters. */
  if (todo > 0) {
    if (*d < de) *(*d)++ = 'x';
    if (*d < de) *(*d)++ = 'n';
    if (*d < de) *(*d)++ = '-';
    if (*d < de) *(*d)++ = '-';
  }

  /* Write ASCII characters. */
  x = 0;
  s = ss;
  while (s < se) {
    c = uv__utf8_decode1(&s, se);
    assert(c != -1u);

    if (c > 127)
      continue;

    if (*d < de)
  while (todo > 0) {
    m = -1;
    s = ss;

    while (s < se) {
      c = uv__utf8_decode1(&s, se);
      assert(c != -1u);

      if (c >= n)
        if (c < m)
          m = c;
    }

    x = m - n;
    y = h + 1;

    n = m;

    s = ss;
    while (s < se) {
      c = uv__utf8_decode1(&s, se);
      assert(c != -1u);

      if (c < n)
        if (++delta == 0)
          return UV_E2BIG;  /* Overflow. */

  return 0;
}

long uv__idna_toascii(const char* s, const char* se, char* d, char* de) {
  const char* si;
  const char* st;
  unsigned c;

  ds = d;

  si = s;
  while (si < se) {
    st = si;
    c = uv__utf8_decode1(&si, se);

    if (c == -1u)
      return UV_EINVAL;

    if (c != '.')
      if (c != 0x3002)  /* 。 */
        if (c != 0xFF0E)  /* ． */
          if (c != 0xFF61)  /* ｡ */