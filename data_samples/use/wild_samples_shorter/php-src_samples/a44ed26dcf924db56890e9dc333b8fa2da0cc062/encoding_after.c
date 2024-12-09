#include "file.h"

#ifndef	lint
FILE_RCSID("@(#)$File: encoding.c,v 1.10 2014/09/11 12:08:52 christos Exp $")
#endif	/* lint */

#include "magic.h"
#include <string.h>
		*code_mime = "utf-8";
	} else if (file_looks_utf8(buf, nbytes, *ubuf, ulen) > 1) {
		DPRINTF(("utf8 %" SIZE_T_FORMAT "u\n", *ulen));
		*code = "UTF-8 Unicode";
		*code_mime = "utf-8";
	} else if ((ucs_type = looks_ucs16(buf, nbytes, *ubuf, ulen)) != 0) {
		if (ucs_type == 1) {
#define X 3   /* character appears in non-ISO extended ASCII (Mac, IBM PC) */

private char text_chars[256] = {
	/*                  BEL BS HT LF VT FF CR    */
	F, F, F, F, F, F, F, T, T, T, T, T, T, T, F, F,  /* 0x0X */
	/*                              ESC          */
	F, F, F, F, F, F, F, F, F, F, F, T, F, F, F, F,  /* 0x1X */
	T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T,  /* 0x2X */
	T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T,  /* 0x3X */