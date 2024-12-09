/*
 * is_tar() -- figure out whether file is a tar archive.
 *
 * Stolen (by the author!) from the public domain tar program:
 * Public Domain version written 26 Aug 1985 John Gilmore (ihnp4!hoptoad!gnu).
 *
 * @(#)list.c 1.18 9/23/86 Public Domain - gnu
 *
#include "file.h"

#ifndef lint
FILE_RCSID("@(#)$File: is_tar.c,v 1.47 2022/09/13 18:46:07 christos Exp $")
#endif

#include "magic.h"
#include <string.h>

#define	isodigit(c)	( ((c) >= '0') && ((c) <= '7') )

private int is_tar(const unsigned char *, size_t);
private int from_oct(const char *, size_t);	/* Decode octal number */

static const char tartype[][32] = {	/* should be equal to messages */
	"tar archive",			/* found in ../magic/Magdir/archive */
	"POSIX tar archive",
	"POSIX tar archive (GNU)",	/*  */
};

protected int
file_is_tar(struct magic_set *ms, const struct buffer *b)
{
	const unsigned char *buf = CAST(const unsigned char *, b->fbuf);
	size_t nbytes = b->flen;
 *	2 for Unix Std (POSIX) tar file,
 *	3 for GNU tar file.
 */
private int
is_tar(const unsigned char *buf, size_t nbytes)
{
	static const char gpkg_match[] = "/gpkg-1";

 *
 * Result is -1 if the field is invalid (all blank, or non-octal).
 */
private int
from_oct(const char *where, size_t digs)
{
	int	value;
