#include "file.h"

#ifndef lint
FILE_RCSID("@(#)$File: is_csv.c,v 1.7 2022/05/28 00:44:22 christos Exp $")
#endif

#include <string.h>
#include "magic.h"
			break;
		}
	}
	return tf && nl > 2;
}

#ifndef TEST
int
file_is_csv(struct magic_set *ms, const struct buffer *b, int looks_text)
{
	const unsigned char *uc = CAST(const unsigned char *, b->fbuf);
	const unsigned char *ue = uc + b->flen;
	int mime = ms->flags & MAGIC_MIME;
		return 1;
	}

	if (file_printf(ms, "CSV text") == -1)
		return -1;

	return 1;
}
int
main(int argc, char *argv[])
{
	int fd, rv;
	struct stat st;
	unsigned char *p;

	if ((fd = open(argv[1], O_RDONLY)) == -1)