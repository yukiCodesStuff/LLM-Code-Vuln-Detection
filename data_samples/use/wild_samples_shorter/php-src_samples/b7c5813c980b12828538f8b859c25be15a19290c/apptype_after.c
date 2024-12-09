/*
 * Adapted from: apptype.c, Written by Eberhard Mattes and put into the
 * file_public domain
 *
 * Notes: 1. Qualify the filename so that DosQueryAppType does not do extraneous
 * searches.
 *
#include "file.h"

#ifndef	lint
FILE_RCSID("@(#)$File: apptype.c,v 1.17 2022/12/26 17:31:14 christos Exp $")
#endif /* lint */

#include <stdlib.h>
#include <string.h>
#include <os2.h>
typedef ULONG   APPTYPE;

file_protected int
file_os2_apptype(struct magic_set *ms, const char *fn, const void *buf,
    size_t nb)
{
	APPTYPE         rc, type;
			return -1;
	} else if (type & FAPPTYP_DLL) {
		if (type & FAPPTYP_PROTDLL)
			if (file_printf(ms, "file_protected ") == -1)
				return -1;
		if (file_printf(ms, "DLL") == -1)
			return -1;
	} else if (type & (FAPPTYP_WINDOWSREAL | FAPPTYP_WINDOWSPROT)) {