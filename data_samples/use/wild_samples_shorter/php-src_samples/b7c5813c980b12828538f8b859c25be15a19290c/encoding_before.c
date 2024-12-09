#include "file.h"

#ifndef	lint
FILE_RCSID("@(#)$File: encoding.c,v 1.39 2022/09/13 18:46:07 christos Exp $")
#endif	/* lint */

#include "magic.h"
#include <string.h>
#include <stdlib.h>


private int looks_ascii(const unsigned char *, size_t, file_unichar_t *,
    size_t *);
private int looks_utf8_with_BOM(const unsigned char *, size_t, file_unichar_t *,
    size_t *);
private int looks_utf7(const unsigned char *, size_t, file_unichar_t *,
    size_t *);
private int looks_ucs16(const unsigned char *, size_t, file_unichar_t *,
    size_t *);
private int looks_ucs32(const unsigned char *, size_t, file_unichar_t *,
    size_t *);
private int looks_latin1(const unsigned char *, size_t, file_unichar_t *,
    size_t *);
private int looks_extended(const unsigned char *, size_t, file_unichar_t *,
    size_t *);
private void from_ebcdic(const unsigned char *, size_t, unsigned char *);

#ifdef DEBUG_ENCODING
#define DPRINTF(a) printf a
#else
 * the text converted into one-file_unichar_t-per-character Unicode in
 * ubuf, and the number of characters converted in ulen.
 */
protected int
file_encoding(struct magic_set *ms, const struct buffer *b,
    file_unichar_t **ubuf, size_t *ulen, const char **code,
    const char **code_mime, const char **type)
{
#define I 2   /* character appears in ISO-8859 text */
#define X 3   /* character appears in non-ISO extended ASCII (Mac, IBM PC) */

private char text_chars[256] = {
	/*                  BEL BS HT LF VT FF CR    */
	F, F, F, F, F, F, F, T, T, T, T, T, T, T, F, F,  /* 0x0X */
	/*                              ESC          */
	F, F, F, F, F, F, F, F, F, F, F, T, F, F, F, F,  /* 0x1X */
};

#define LOOKS(NAME, COND) \
private int \
looks_ ## NAME(const unsigned char *buf, size_t nbytes, file_unichar_t *ubuf, \
    size_t *ulen) \
{ \
	size_t i; \
	{ LOCB, 0x8F },
};

protected int
file_looks_utf8(const unsigned char *buf, size_t nbytes, file_unichar_t *ubuf,
    size_t *ulen)
{
	size_t i;
 * BOM, return -1; otherwise return the result of looks_utf8 on the
 * rest of the text.
 */
private int
looks_utf8_with_BOM(const unsigned char *buf, size_t nbytes,
    file_unichar_t *ubuf, size_t *ulen)
{
	if (nbytes > 3 && buf[0] == 0xef && buf[1] == 0xbb && buf[2] == 0xbf)
		return -1;
}

private int
looks_utf7(const unsigned char *buf, size_t nbytes, file_unichar_t *ubuf,
    size_t *ulen)
{
	if (nbytes > 4 && buf[0] == '+' && buf[1] == '/' && buf[2] == 'v')
#define UCS16_HISURR(c) ((c) >= 0xd800 && (c) <= 0xdbff)
#define UCS16_LOSURR(c) ((c) >= 0xdc00 && (c) <= 0xdfff)

private int
looks_ucs16(const unsigned char *bf, size_t nbytes, file_unichar_t *ubf,
    size_t *ulen)
{
	int bigend;
	return 1 + bigend;
}

private int
looks_ucs32(const unsigned char *bf, size_t nbytes, file_unichar_t *ubf,
    size_t *ulen)
{
	int bigend;
 * between old-style and internationalized examples of text.
 */

private unsigned char ebcdic_to_ascii[] = {
  0,   1,   2,   3, 156,   9, 134, 127, 151, 141, 142,  11,  12,  13,  14,  15,
 16,  17,  18,  19, 157, 133,   8, 135,  24,  25, 146, 143,  28,  29,  30,  31,
128, 129, 130, 131, 132,  10,  23,  27, 136, 137, 138, 139, 140,   5,   6,   7,
144, 145,  22, 147, 148, 149, 150,   4, 152, 153, 154, 155,  20,  21, 158,  26,
 * cases for the NEL character can be taken out of the code.
 */

private unsigned char ebcdic_1047_to_8859[] = {
0x00,0x01,0x02,0x03,0x9C,0x09,0x86,0x7F,0x97,0x8D,0x8E,0x0B,0x0C,0x0D,0x0E,0x0F,
0x10,0x11,0x12,0x13,0x9D,0x0A,0x08,0x87,0x18,0x19,0x92,0x8F,0x1C,0x1D,0x1E,0x1F,
0x80,0x81,0x82,0x83,0x84,0x85,0x17,0x1B,0x88,0x89,0x8A,0x8B,0x8C,0x05,0x06,0x07,
0x90,0x91,0x16,0x93,0x94,0x95,0x96,0x04,0x98,0x99,0x9A,0x9B,0x14,0x15,0x9E,0x1A,
/*
 * Copy buf[0 ... nbytes-1] into out[], translating EBCDIC to ASCII.
 */
private void
from_ebcdic(const unsigned char *buf, size_t nbytes, unsigned char *out)
{
	size_t i;
