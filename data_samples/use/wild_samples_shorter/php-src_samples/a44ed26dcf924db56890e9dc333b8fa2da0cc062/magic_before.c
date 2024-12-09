#include "file.h"

#ifndef	lint
FILE_RCSID("@(#)$File: magic.c,v 1.81 2013/11/29 15:42:51 christos Exp $")
#endif	/* lint */

#include "magic.h"

			tmppath = NULL; \
		} \
	} while (/*CONSTCOND*/0)

	if (default_magic) {
		free(default_magic);
		default_magic = NULL;
	}
private int
unreadable_info(struct magic_set *ms, mode_t md, const char *file)
{
	/* We cannot open it, but we were able to stat it. */
	if (access(file, W_OK) == 0)
		if (file_printf(ms, "writable, ") == -1)
			return -1;
	if (access(file, X_OK) == 0)
		if (file_printf(ms, "executable, ") == -1)
			return -1;
	if (S_ISREG(md))
		if (file_printf(ms, "regular file, ") == -1)
			return -1;
	if (file_printf(ms, "no read permission") == -1)
		return NULL;
	/*
	 * The main work is done here!
	 * We have the file name and/or the data buffer to be identified.
	 */
	if (file_buffer(ms, NULL, NULL, buf, nb) == -1) {
		return NULL;
	}
{
	return MAGIC_VERSION;
}