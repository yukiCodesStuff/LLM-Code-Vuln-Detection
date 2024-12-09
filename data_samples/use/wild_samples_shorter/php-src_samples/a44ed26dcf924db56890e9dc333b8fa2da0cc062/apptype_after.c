/*
 * Adapted from: apptype.c, Written by Eberhard Mattes and put into the
 * public domain
 * 
 * Notes: 1. Qualify the filename so that DosQueryAppType does not do extraneous
 * searches.
 * 
 * 2. DosQueryAppType will return FAPPTYP_DOS on a file ending with ".com"
 * (other than an OS/2 exe or Win exe with this name). Eberhard Mattes
 * remarks Tue, 6 Apr 93: Moreover, it reports the type of the (new and very
 * bug ridden) Win Emacs as "OS/2 executable".
 * 
 * 3. apptype() uses the filename if given, otherwise a tmp file is created with
 * the contents of buf. If buf is not the complete file, apptype can
 * incorrectly identify the exe type. The "-z" option of "file" is the reason
 * for this ugly code.

/*
 * amai: Darrel Hankerson did the changes described here.
 * 
 * It remains to check the validity of comments (2.) since it's referred to an
 * "old" OS/2 version.
 * 
 */

#include "file.h"

#ifndef	lint
FILE_RCSID("@(#)$File: apptype.c,v 1.12 2011/08/28 07:03:27 christos Exp $")
#endif /* lint */

#include <stdlib.h>
#include <string.h>