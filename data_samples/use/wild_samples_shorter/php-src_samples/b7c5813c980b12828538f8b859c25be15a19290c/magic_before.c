#include "file.h"

#ifndef	lint
FILE_RCSID("@(#)$File: magic.c,v 1.117 2021/12/06 15:33:00 christos Exp $")
#endif	/* lint */

#include "magic.h"

# undef S_IFIFO
#endif

private int unreadable_info(struct magic_set *, mode_t, const char *);
private const char *file_or_stream(struct magic_set *, const char *, php_stream *);

#ifndef	STDIN_FILENO
#define	STDIN_FILENO	0
#endif

public struct magic_set *
magic_open(int flags)
{
	return file_ms_alloc(flags);
}

private int
unreadable_info(struct magic_set *ms, mode_t md, const char *file)
{
	if (file) {
		/* We cannot open it, but we were able to stat it. */
	return 0;
}

public void
magic_close(struct magic_set *ms)
{
	if (ms == NULL)
		return;
/*
 * load a magic file
 */
public int
magic_load(struct magic_set *ms, const char *magicfile)
{
	if (ms == NULL)
		return -1;
	return file_apprentice(ms, magicfile, FILE_LOAD);
}

public int
magic_compile(struct magic_set *ms, const char *magicfile)
{
	if (ms == NULL)
		return -1;
	return file_apprentice(ms, magicfile, FILE_COMPILE);
}

public int
magic_check(struct magic_set *ms, const char *magicfile)
{
	if (ms == NULL)
		return -1;
	return file_apprentice(ms, magicfile, FILE_CHECK);
}

public int
magic_list(struct magic_set *ms, const char *magicfile)
{
	if (ms == NULL)
		return -1;
/*
 * find type of descriptor
 */
public const char *
magic_descriptor(struct magic_set *ms, int fd)
{
	if (ms == NULL)
		return NULL;
/*
 * find type of named file
 */
public const char *
magic_file(struct magic_set *ms, const char *inname)
{
	if (ms == NULL)
		return NULL;
	return file_or_stream(ms, inname, NULL);
}

public const char *
magic_stream(struct magic_set *ms, php_stream *stream)
{
	if (ms == NULL)
		return NULL;
	return file_or_stream(ms, NULL, stream);
}

private const char *
file_or_stream(struct magic_set *ms, const char *inname, php_stream *stream)
{
	int	rv = -1;
	unsigned char *buf;
}


public const char *
magic_buffer(struct magic_set *ms, const void *buf, size_t nb)
{
	if (ms == NULL)
		return NULL;
}
#endif

public const char *
magic_error(struct magic_set *ms)
{
	if (ms == NULL)
		return "Magic database is not open";
	return (ms->event_flags & EVENT_HAD_ERR) ? ms->o.buf : NULL;
}

public int
magic_errno(struct magic_set *ms)
{
	if (ms == NULL)
		return EINVAL;
	return (ms->event_flags & EVENT_HAD_ERR) ? ms->error : 0;
}

public int
magic_getflags(struct magic_set *ms)
{
	if (ms == NULL)
		return -1;
	return ms->flags;
}

public int
magic_setflags(struct magic_set *ms, int flags)
{
	if (ms == NULL)
		return -1;
	return 0;
}

public int
magic_version(void)
{
	return MAGIC_VERSION;
}

public int
magic_setparam(struct magic_set *ms, int param, const void *val)
{
	if (ms == NULL)
		return -1;
	case MAGIC_PARAM_ELF_SHNUM_MAX:
		ms->elf_shnum_max = CAST(uint16_t, *CAST(const size_t *, val));
		return 0;
	case MAGIC_PARAM_ELF_NOTES_MAX:
		ms->elf_notes_max = CAST(uint16_t, *CAST(const size_t *, val));
		return 0;
	case MAGIC_PARAM_REGEX_MAX:
	}
}

public int
magic_getparam(struct magic_set *ms, int param, void *val)
{
	if (ms == NULL)
		return -1;
	case MAGIC_PARAM_ELF_SHNUM_MAX:
		*CAST(size_t *, val) = ms->elf_shnum_max;
		return 0;
	case MAGIC_PARAM_ELF_NOTES_MAX:
		*CAST(size_t *, val) = ms->elf_notes_max;
		return 0;
	case MAGIC_PARAM_REGEX_MAX: