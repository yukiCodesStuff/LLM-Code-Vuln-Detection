#include "file.h"

#ifndef	lint
FILE_RCSID("@(#)$File: ascmagic.c,v 1.88 2014/02/12 23:20:53 christos Exp $")
#endif	/* lint */

#include "magic.h"
#include <string.h>
		    == NULL)
			goto done;
		if ((rv = file_softmagic(ms, utf8_buf,
		    (size_t)(utf8_end - utf8_buf), 0, TEXTTEST, text)) == 0)
			rv = -1;
	}

	/* Now try to discover other details about the file. */