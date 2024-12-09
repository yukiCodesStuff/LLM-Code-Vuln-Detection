#include "file.h"

#ifndef	lint
FILE_RCSID("@(#)$File: ascmagic.c,v 1.90 2014/11/28 02:35:05 christos Exp $")
#endif	/* lint */

#include "magic.h"
#include <string.h>
		    == NULL)
			goto done;
		if ((rv = file_softmagic(ms, utf8_buf,
		    (size_t)(utf8_end - utf8_buf), 0, NULL,
		    TEXTTEST, text)) == 0)
			rv = -1;
	}

	/* Now try to discover other details about the file. */