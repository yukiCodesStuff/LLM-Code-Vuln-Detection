/*
 * Copyright (c) Christos Zoulas 2003.
 * All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice immediately at the beginning of the file, without modification,
 *    this list of conditions, and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "file.h"

#ifndef	lint
FILE_RCSID("@(#)$File: magic.c,v 1.121 2023/02/09 17:45:19 christos Exp $")
#endif	/* lint */

#include "magic.h"

#include <stdlib.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <string.h>
#ifdef QUICK
#include <sys/mman.h>
#endif
#include <limits.h>	/* for PIPE_BUF */

#if defined(HAVE_UTIMES)
# include <sys/time.h>
#elif defined(HAVE_UTIME)
# if defined(HAVE_SYS_UTIME_H)
#  include <sys/utime.h>
# elif defined(HAVE_UTIME_H)
#  include <utime.h>
# endif
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>	/* for read() */
#endif

#ifndef PIPE_BUF
/* Get the PIPE_BUF from pathconf */
#ifdef _PC_PIPE_BUF
#define PIPE_BUF pathconf(".", _PC_PIPE_BUF)
#else
#define PIPE_BUF 512
#endif
#endif

#ifdef PHP_WIN32
# undef S_IFLNK
# undef S_IFIFO
#endif

file_private int unreadable_info(struct magic_set *, mode_t, const char *);
file_private const char *file_or_stream(struct magic_set *, const char *, php_stream *);

#ifndef	STDIN_FILENO
#define	STDIN_FILENO	0
#endif

file_public struct magic_set *
magic_open(int flags)
{
	return file_ms_alloc(flags);
}
/*
 * Copyright (c) Christos Zoulas 2003.
 * All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice immediately at the beginning of the file, without modification,
 *    this list of conditions, and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "file.h"

#ifndef	lint
FILE_RCSID("@(#)$File: magic.c,v 1.121 2023/02/09 17:45:19 christos Exp $")
#endif	/* lint */

#include "magic.h"

#include <stdlib.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <string.h>
#ifdef QUICK
#include <sys/mman.h>
#endif
#include <limits.h>	/* for PIPE_BUF */

#if defined(HAVE_UTIMES)
# include <sys/time.h>
#elif defined(HAVE_UTIME)
# if defined(HAVE_SYS_UTIME_H)
#  include <sys/utime.h>
# elif defined(HAVE_UTIME_H)
#  include <utime.h>
# endif
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>	/* for read() */
#endif

#ifndef PIPE_BUF
/* Get the PIPE_BUF from pathconf */
#ifdef _PC_PIPE_BUF
#define PIPE_BUF pathconf(".", _PC_PIPE_BUF)
#else
#define PIPE_BUF 512
#endif
#endif

#ifdef PHP_WIN32
# undef S_IFLNK
# undef S_IFIFO
#endif

file_private int unreadable_info(struct magic_set *, mode_t, const char *);
file_private const char *file_or_stream(struct magic_set *, const char *, php_stream *);

#ifndef	STDIN_FILENO
#define	STDIN_FILENO	0
#endif

file_public struct magic_set *
magic_open(int flags)
{
	return file_ms_alloc(flags);
}
		{
			const char *p = strrchr(file, '.');
			if (p && (stricmp(p, ".exe")
				  || stricmp(p, ".dll")
				  || stricmp(p, ".bat")
				  || stricmp(p, ".cmd")))
				if (file_printf(ms, "writable, ") == -1)
					return -1;
		}
#endif
	}
	if (S_ISREG(md))
		if (file_printf(ms, "regular file, ") == -1)
			return -1;
	if (file_printf(ms, "no read permission") == -1)
		return -1;
	return 0;
}

file_public void
magic_close(struct magic_set *ms)
{
	if (ms == NULL)
		return;
	file_ms_free(ms);
}

/*
 * load a magic file
 */
file_public int
magic_load(struct magic_set *ms, const char *magicfile)
{
	if (ms == NULL)
		return -1;
	return file_apprentice(ms, magicfile, FILE_LOAD);
}

file_public int
magic_compile(struct magic_set *ms, const char *magicfile)
{
	if (ms == NULL)
		return -1;
	return file_apprentice(ms, magicfile, FILE_COMPILE);
}

file_public int
magic_check(struct magic_set *ms, const char *magicfile)
{
	if (ms == NULL)
		return -1;
	return file_apprentice(ms, magicfile, FILE_CHECK);
}

file_public int
magic_list(struct magic_set *ms, const char *magicfile)
{
	if (ms == NULL)
		return -1;
	return file_apprentice(ms, magicfile, FILE_LIST);
}

#ifndef COMPILE_ONLY

/*
 * find type of descriptor
 */
file_public const char *
magic_descriptor(struct magic_set *ms, int fd)
{
	if (ms == NULL)
		return NULL;
	return file_or_stream(ms, NULL, NULL);
}

/*
 * find type of named file
 */
file_public const char *
magic_file(struct magic_set *ms, const char *inname)
{
	if (ms == NULL)
		return NULL;
	return file_or_stream(ms, inname, NULL);
}

file_public const char *
magic_stream(struct magic_set *ms, php_stream *stream)
{
	if (ms == NULL)
		return NULL;
	return file_or_stream(ms, NULL, stream);
}

file_private const char *
file_or_stream(struct magic_set *ms, const char *inname, php_stream *stream)
{
	int	rv = -1;
	unsigned char *buf;
	zend_stat_t   sb = {0};
	ssize_t nbytes = 0;	/* number of bytes read from a datafile */
	int no_in_stream = 0;

	if (file_reset(ms, 1) == -1)
		goto out;

	/*
	 * one extra for terminating '\0', and
	 * some overlapping space for matches near EOF
	 */
#define SLOP (1 + sizeof(union VALUETYPE))
	if ((buf = CAST(unsigned char *, emalloc(ms->bytes_max + SLOP))) == NULL)
		return NULL;

	switch (file_fsmagic(ms, inname, &sb)) {
	case -1:		/* error */
		goto done;
	case 0:			/* nothing found */
		break;
	default:		/* matched it and printed type */
		rv = 0;
		goto done;
	}
{
	if (ms == NULL)
		return;
	file_ms_free(ms);
}

/*
 * load a magic file
 */
file_public int
magic_load(struct magic_set *ms, const char *magicfile)
{
	if (ms == NULL)
		return -1;
	return file_apprentice(ms, magicfile, FILE_LOAD);
}
{
	if (ms == NULL)
		return -1;
	return file_apprentice(ms, magicfile, FILE_LIST);
}

#ifndef COMPILE_ONLY

/*
 * find type of descriptor
 */
file_public const char *
magic_descriptor(struct magic_set *ms, int fd)
{
	if (ms == NULL)
		return NULL;
	return file_or_stream(ms, NULL, NULL);
}
{
	if (ms == NULL)
		return NULL;
	return file_or_stream(ms, NULL, NULL);
}

/*
 * find type of named file
 */
file_public const char *
magic_file(struct magic_set *ms, const char *inname)
{
	if (ms == NULL)
		return NULL;
	return file_or_stream(ms, inname, NULL);
}
	return rv == 0 ? file_getbuffer(ms) : NULL;
}


file_public const char *
magic_buffer(struct magic_set *ms, const void *buf, size_t nb)
{
	if (ms == NULL)
		return NULL;
	if (file_reset(ms, 1) == -1)
		return NULL;
	/*
	 * The main work is done here!
	 * We have the file name and/or the data buffer to be identified.
	 */
	if (file_buffer(ms, NULL, NULL, NULL, buf, nb) == -1) {
		return NULL;
	}
	return file_getbuffer(ms);
}
	if (file_buffer(ms, NULL, NULL, NULL, buf, nb) == -1) {
		return NULL;
	}
	return file_getbuffer(ms);
}
#endif

file_public const char *
magic_error(struct magic_set *ms)
{
	if (ms == NULL)
		return "Magic database is not open";
	return (ms->event_flags & EVENT_HAD_ERR) ? ms->o.buf : NULL;
}
{
	if (ms == NULL)
		return -1;

	return ms->flags;
}

file_public int
magic_setflags(struct magic_set *ms, int flags)
{
	if (ms == NULL)
		return -1;
#if !defined(HAVE_UTIME) && !defined(HAVE_UTIMES)
	if (flags & MAGIC_PRESERVE_ATIME)
		return -1;
#endif
	ms->flags = flags;
	return 0;
}

file_public int
magic_version(void)
{
	return MAGIC_VERSION;
}

file_public int
magic_setparam(struct magic_set *ms, int param, const void *val)
{
	if (ms == NULL)
		return -1;
	switch (param) {
	case MAGIC_PARAM_INDIR_MAX:
		ms->indir_max = CAST(uint16_t, *CAST(const size_t *, val));
		return 0;
	case MAGIC_PARAM_NAME_MAX:
		ms->name_max = CAST(uint16_t, *CAST(const size_t *, val));
		return 0;
	case MAGIC_PARAM_ELF_PHNUM_MAX:
		ms->elf_phnum_max = CAST(uint16_t, *CAST(const size_t *, val));
		return 0;
	case MAGIC_PARAM_ELF_SHNUM_MAX:
		ms->elf_shnum_max = CAST(uint16_t, *CAST(const size_t *, val));
		return 0;
	case MAGIC_PARAM_ELF_SHSIZE_MAX:
		ms->elf_shsize_max = *CAST(const size_t *, val);
		return 0;
	case MAGIC_PARAM_ELF_NOTES_MAX:
		ms->elf_notes_max = CAST(uint16_t, *CAST(const size_t *, val));
		return 0;
	case MAGIC_PARAM_REGEX_MAX:
		ms->regex_max = CAST(uint16_t, *CAST(const size_t *, val));
		return 0;
	case MAGIC_PARAM_BYTES_MAX:
		ms->bytes_max = *CAST(const size_t *, val);
		return 0;
	case MAGIC_PARAM_ENCODING_MAX:
		ms->encoding_max = *CAST(const size_t *, val);
		return 0;
	default:
		errno = EINVAL;
		return -1;
	}
{
	if (ms == NULL)
		return -1;
#if !defined(HAVE_UTIME) && !defined(HAVE_UTIMES)
	if (flags & MAGIC_PRESERVE_ATIME)
		return -1;
#endif
	ms->flags = flags;
	return 0;
}

file_public int
magic_version(void)
{
	return MAGIC_VERSION;
}

file_public int
magic_setparam(struct magic_set *ms, int param, const void *val)
{
	if (ms == NULL)
		return -1;
	switch (param) {
	case MAGIC_PARAM_INDIR_MAX:
		ms->indir_max = CAST(uint16_t, *CAST(const size_t *, val));
		return 0;
	case MAGIC_PARAM_NAME_MAX:
		ms->name_max = CAST(uint16_t, *CAST(const size_t *, val));
		return 0;
	case MAGIC_PARAM_ELF_PHNUM_MAX:
		ms->elf_phnum_max = CAST(uint16_t, *CAST(const size_t *, val));
		return 0;
	case MAGIC_PARAM_ELF_SHNUM_MAX:
		ms->elf_shnum_max = CAST(uint16_t, *CAST(const size_t *, val));
		return 0;
	case MAGIC_PARAM_ELF_SHSIZE_MAX:
		ms->elf_shsize_max = *CAST(const size_t *, val);
		return 0;
	case MAGIC_PARAM_ELF_NOTES_MAX:
		ms->elf_notes_max = CAST(uint16_t, *CAST(const size_t *, val));
		return 0;
	case MAGIC_PARAM_REGEX_MAX:
		ms->regex_max = CAST(uint16_t, *CAST(const size_t *, val));
		return 0;
	case MAGIC_PARAM_BYTES_MAX:
		ms->bytes_max = *CAST(const size_t *, val);
		return 0;
	case MAGIC_PARAM_ENCODING_MAX:
		ms->encoding_max = *CAST(const size_t *, val);
		return 0;
	default:
		errno = EINVAL;
		return -1;
	}
	switch (param) {
	case MAGIC_PARAM_INDIR_MAX:
		ms->indir_max = CAST(uint16_t, *CAST(const size_t *, val));
		return 0;
	case MAGIC_PARAM_NAME_MAX:
		ms->name_max = CAST(uint16_t, *CAST(const size_t *, val));
		return 0;
	case MAGIC_PARAM_ELF_PHNUM_MAX:
		ms->elf_phnum_max = CAST(uint16_t, *CAST(const size_t *, val));
		return 0;
	case MAGIC_PARAM_ELF_SHNUM_MAX:
		ms->elf_shnum_max = CAST(uint16_t, *CAST(const size_t *, val));
		return 0;
	case MAGIC_PARAM_ELF_SHSIZE_MAX:
		ms->elf_shsize_max = *CAST(const size_t *, val);
		return 0;
	case MAGIC_PARAM_ELF_NOTES_MAX:
		ms->elf_notes_max = CAST(uint16_t, *CAST(const size_t *, val));
		return 0;
	case MAGIC_PARAM_REGEX_MAX:
		ms->regex_max = CAST(uint16_t, *CAST(const size_t *, val));
		return 0;
	case MAGIC_PARAM_BYTES_MAX:
		ms->bytes_max = *CAST(const size_t *, val);
		return 0;
	case MAGIC_PARAM_ENCODING_MAX:
		ms->encoding_max = *CAST(const size_t *, val);
		return 0;
	default:
		errno = EINVAL;
		return -1;
	}
}

file_public int
magic_getparam(struct magic_set *ms, int param, void *val)
{
	if (ms == NULL)
		return -1;
	switch (param) {
	case MAGIC_PARAM_INDIR_MAX:
		*CAST(size_t *, val) = ms->indir_max;
		return 0;
	case MAGIC_PARAM_NAME_MAX:
		*CAST(size_t *, val) = ms->name_max;
		return 0;
	case MAGIC_PARAM_ELF_PHNUM_MAX:
		*CAST(size_t *, val) = ms->elf_phnum_max;
		return 0;
	case MAGIC_PARAM_ELF_SHNUM_MAX:
		*CAST(size_t *, val) = ms->elf_shnum_max;
		return 0;
	case MAGIC_PARAM_ELF_SHSIZE_MAX:
		*CAST(size_t *, val) = ms->elf_shsize_max;
		return 0;
	case MAGIC_PARAM_ELF_NOTES_MAX:
		*CAST(size_t *, val) = ms->elf_notes_max;
		return 0;
	case MAGIC_PARAM_REGEX_MAX:
		*CAST(size_t *, val) = ms->regex_max;
		return 0;
	case MAGIC_PARAM_BYTES_MAX:
		*CAST(size_t *, val) = ms->bytes_max;
		return 0;
	case MAGIC_PARAM_ENCODING_MAX:
		*CAST(size_t *, val) = ms->encoding_max;
		return 0;
	default:
		errno = EINVAL;
		return -1;
	}
}
	switch (param) {
	case MAGIC_PARAM_INDIR_MAX:
		ms->indir_max = CAST(uint16_t, *CAST(const size_t *, val));
		return 0;
	case MAGIC_PARAM_NAME_MAX:
		ms->name_max = CAST(uint16_t, *CAST(const size_t *, val));
		return 0;
	case MAGIC_PARAM_ELF_PHNUM_MAX:
		ms->elf_phnum_max = CAST(uint16_t, *CAST(const size_t *, val));
		return 0;
	case MAGIC_PARAM_ELF_SHNUM_MAX:
		ms->elf_shnum_max = CAST(uint16_t, *CAST(const size_t *, val));
		return 0;
	case MAGIC_PARAM_ELF_SHSIZE_MAX:
		ms->elf_shsize_max = *CAST(const size_t *, val);
		return 0;
	case MAGIC_PARAM_ELF_NOTES_MAX:
		ms->elf_notes_max = CAST(uint16_t, *CAST(const size_t *, val));
		return 0;
	case MAGIC_PARAM_REGEX_MAX:
		ms->regex_max = CAST(uint16_t, *CAST(const size_t *, val));
		return 0;
	case MAGIC_PARAM_BYTES_MAX:
		ms->bytes_max = *CAST(const size_t *, val);
		return 0;
	case MAGIC_PARAM_ENCODING_MAX:
		ms->encoding_max = *CAST(const size_t *, val);
		return 0;
	default:
		errno = EINVAL;
		return -1;
	}
}

file_public int
magic_getparam(struct magic_set *ms, int param, void *val)
{
	if (ms == NULL)
		return -1;
	switch (param) {
	case MAGIC_PARAM_INDIR_MAX:
		*CAST(size_t *, val) = ms->indir_max;
		return 0;
	case MAGIC_PARAM_NAME_MAX:
		*CAST(size_t *, val) = ms->name_max;
		return 0;
	case MAGIC_PARAM_ELF_PHNUM_MAX:
		*CAST(size_t *, val) = ms->elf_phnum_max;
		return 0;
	case MAGIC_PARAM_ELF_SHNUM_MAX:
		*CAST(size_t *, val) = ms->elf_shnum_max;
		return 0;
	case MAGIC_PARAM_ELF_SHSIZE_MAX:
		*CAST(size_t *, val) = ms->elf_shsize_max;
		return 0;
	case MAGIC_PARAM_ELF_NOTES_MAX:
		*CAST(size_t *, val) = ms->elf_notes_max;
		return 0;
	case MAGIC_PARAM_REGEX_MAX:
		*CAST(size_t *, val) = ms->regex_max;
		return 0;
	case MAGIC_PARAM_BYTES_MAX:
		*CAST(size_t *, val) = ms->bytes_max;
		return 0;
	case MAGIC_PARAM_ENCODING_MAX:
		*CAST(size_t *, val) = ms->encoding_max;
		return 0;
	default:
		errno = EINVAL;
		return -1;
	}
	switch (param) {
	case MAGIC_PARAM_INDIR_MAX:
		*CAST(size_t *, val) = ms->indir_max;
		return 0;
	case MAGIC_PARAM_NAME_MAX:
		*CAST(size_t *, val) = ms->name_max;
		return 0;
	case MAGIC_PARAM_ELF_PHNUM_MAX:
		*CAST(size_t *, val) = ms->elf_phnum_max;
		return 0;
	case MAGIC_PARAM_ELF_SHNUM_MAX:
		*CAST(size_t *, val) = ms->elf_shnum_max;
		return 0;
	case MAGIC_PARAM_ELF_SHSIZE_MAX:
		*CAST(size_t *, val) = ms->elf_shsize_max;
		return 0;
	case MAGIC_PARAM_ELF_NOTES_MAX:
		*CAST(size_t *, val) = ms->elf_notes_max;
		return 0;
	case MAGIC_PARAM_REGEX_MAX:
		*CAST(size_t *, val) = ms->regex_max;
		return 0;
	case MAGIC_PARAM_BYTES_MAX:
		*CAST(size_t *, val) = ms->bytes_max;
		return 0;
	case MAGIC_PARAM_ENCODING_MAX:
		*CAST(size_t *, val) = ms->encoding_max;
		return 0;
	default:
		errno = EINVAL;
		return -1;
	}
}