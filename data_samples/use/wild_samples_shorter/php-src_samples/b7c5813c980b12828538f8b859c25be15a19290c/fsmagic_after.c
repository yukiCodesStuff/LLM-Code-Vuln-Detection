#include "file.h"

#ifndef	lint
FILE_RCSID("@(#)$File: fsmagic.c,v 1.85 2022/12/26 17:31:14 christos Exp $")
#endif	/* lint */

#include "magic.h"
#include <string.h>

# undef S_IFIFO
#endif
file_private int
handle_mime(struct magic_set *ms, int mime, const char *str)
{
	if ((mime & MAGIC_MIME_TYPE)) {
		if (file_printf(ms, "inode/%s", str) == -1)
	return 0;
}

file_protected int
file_fsmagic(struct magic_set *ms, const char *fn, zend_stat_t *sb)
{
	int ret, did = 0;
	int mime = ms->flags & MAGIC_MIME;