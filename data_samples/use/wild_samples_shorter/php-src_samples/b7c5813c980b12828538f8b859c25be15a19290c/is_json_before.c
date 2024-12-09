#include "file.h"

#ifndef lint
FILE_RCSID("@(#)$File: is_json.c,v 1.26 2022/09/13 18:46:07 christos Exp $")
#endif

#include "magic.h"
#else
		return 1;
	if (mime) {
		if (file_printf(ms, "application/%s",
		    jt == 1 ? "json" : "x-ndjason") == -1)
			return -1;
		return 1;
	}
	if (file_printf(ms, "%sJSON text data",
int
main(int argc, char *argv[])
{
	int fd, rv;
	struct stat st;
	unsigned char *p;
	size_t stats[JSON_MAX];
