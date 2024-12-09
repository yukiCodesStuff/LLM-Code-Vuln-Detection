 * Copyright (c) Ian F. Darwin 1986-1995.
 * Software written by Ian F. Darwin and others;
 * maintained 1995-present by Christos Zoulas and others.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE FOR
#include "file.h"

#ifndef	lint
FILE_RCSID("@(#)$File: apprentice.c,v 1.196 2013/11/19 21:01:12 christos Exp $")
#endif	/* lint */

#include "magic.h"
#include "patchlevel.h"
#include <ctype.h>
#include <fcntl.h>

#define	EATAB {while (isascii((unsigned char) *l) && \
		      isspace((unsigned char) *l))  ++l;}
#define LOWCASE(l) (isupper((unsigned char) (l)) ? \
			tolower((unsigned char) (l)) : (l))
#define ALLOC_CHUNK	(size_t)10
#define ALLOC_INCR	(size_t)200

struct magic_entry {
	struct magic *mp;
	uint32_t cont_count;
	uint32_t max_count;
};

struct magic_map {
	void *p;
	size_t len;
	struct magic *magic[MAGIC_SETS];
	uint32_t nmagic[MAGIC_SETS];
};

private size_t apprentice_magic_strength(const struct magic *);
private int apprentice_sort(const void *, const void *);
private void apprentice_list(struct mlist *, int );
private struct magic_map *apprentice_load(struct magic_set *,
    const char *, int);
private struct mlist *mlist_alloc(void);
private void mlist_free(struct mlist *);
private void byteswap(struct magic *, uint32_t);
private uint32_t swap4(uint32_t);
private uint64_t swap8(uint64_t);
private char *mkdbname(struct magic_set *, const char *, int);
private struct magic_map *apprentice_map(struct magic_set *, const char *);
private void apprentice_unmap(struct magic_map *);
private int apprentice_compile(struct magic_set *, struct magic_map *,
    const char *);
private int check_format_type(const char *, int);
{
	struct mlist *ml;

	if ((ml = CAST(struct mlist *, emalloc(sizeof(*ml)))) == NULL)
		return -1;

	ml->map = idx == 0 ? map : NULL;
	ml->magic = map->magic[idx];
	ml->nmagic = map->nmagic[idx];

	mlp->prev->next = ml;
private int
apprentice_1(struct magic_set *ms, const char *fn, int action)
{
	struct mlist *ml;
	struct magic_map *map;
	size_t i;

	if (magicsize != FILE_MAGICSIZE) {
		file_error(ms, 0, "magic element size %lu != %lu",

	if (action == FILE_LIST) {
		for (i = 0; i < MAGIC_SETS; i++) {
			printf("Set %zu:\nBinary patterns:\n", i);
			apprentice_list(ms->mlist[i], BINTEST);
			printf("Text patterns:\n");
			apprentice_list(ms->mlist[i], TEXTTEST);
		}
	}

	return 0;
}

protected void
		ms->mlist[i] = NULL;
	ms->file = "unknown";
	ms->line = 0;
	return ms;
free:
	efree(ms);
	return NULL;
private void
mlist_free(struct mlist *mlist)
{
	struct mlist *ml;

	if (mlist == NULL)
		return;

	for (ml = mlist->next; ml != mlist;) {
		struct mlist *next = ml->next;
		if (ml->map)
			apprentice_unmap(ml->map);
		efree(ml);
		ml = next;
	}
	efree(ml);
}

/* const char *fn: list of magic files and directories */
protected int
	int file_err, errs = -1;
	size_t i;

	file_reset(ms);

/* XXX disabling default magic loading so the compiled in data is used */
#if 0
	if ((fn = magic_getpath(fn, action)) == NULL)
		mlist_free(ms->mlist[i]);
		if ((ms->mlist[i] = mlist_alloc()) == NULL) {
			file_oomem(ms, sizeof(*ms->mlist[i]));
			if (i != 0) {
				--i;
				do
					mlist_free(ms->mlist[i]);
				while (i != 0);
			}
			efree(mfn);
			return -1;
		}
	}
}

/*
 * Get weight of this magic entry, for sorting purposes.
 */
private size_t
apprentice_magic_strength(const struct magic *m)
{
#define MULT 10
	size_t val = 2 * MULT;	/* baseline strength */

	switch (m->type) {
	case FILE_DEFAULT:	/* make sure this sorts last */
		if (m->factor_op != FILE_FACTOR_OP_NONE)
		break;

	case FILE_SEARCH:
	case FILE_REGEX:
		val += m->vallen * MAX(MULT / m->vallen, 1);
		break;

	case FILE_DATE:
	case FILE_LEDATE:
	case FILE_BEDATE:
	case FILE_MEDATE:
		break;

	default:
		val = 0;
		(void)fprintf(stderr, "Bad type %d\n", m->type);
		abort();
	}

	return val;
}

/*
 * Sort callback for sorting entries by "strength" (basically length)
 */
private int
apprentice_sort(const void *a, const void *b)
		return 1;
}

/*
 * Shows sorted patterns list in the order which is used for the matching
 */
private void
apprentice_list(struct mlist *mlist, int mode)
			       *ml->magic[magindex].mimetype == '\0')
				magindex++;

			printf("Strength = %3" SIZE_T_FORMAT "u : %s [%s]\n",
			    apprentice_magic_strength(m),
			    ml->magic[magindex].desc,
			    ml->magic[magindex].mimetype);
		}
	}
			mstart->flag |= BINTEST;
		if (mstart->str_flags & STRING_TEXTTEST)
			mstart->flag |= TEXTTEST;

		if (mstart->flag & (TEXTTEST|BINTEST))
			break;

		/* binary test if pattern is not text */
	}
	if (me.mp)
		(void)addentry(ms, &me, mset);
	php_stream_close(stream);
}

/*
				file_magwarn(ms,
				    "level 0 \"default\" did not sort last");
			}
			return;
		}
	}
}

	struct magic_entry_set mset[MAGIC_SETS];
	php_stream *dir;
	php_stream_dirent d;


	memset(mset, 0, sizeof(mset));
	ms->flags |= MAGIC_CHECK;	/* Enable checks for parsed files */

		magic_entry_free(mset[j].me, mset[j].count);

	if (errs) {
		for (j = 0; j < MAGIC_SETS; j++) {
			if (map->magic[j])
				efree(map->magic[j]);
		}
		efree(map);
		return NULL;
	}
	return map;
}
	if ((ms->flags & MAGIC_CHECK) == 0)
		return 0;

	if (m->type != FILE_PSTRING && (m->str_flags & PSTRING_LEN) != 0) {
		file_magwarn(ms,
		    "'/BHhLl' modifiers are only allowed for pascal strings\n");
		return -1;
	}
}
#endif /* ENABLE_CONDITIONALS */

/*
 * parse one line from magic file, put into magic[index++] if valid
 */
private int
	 */
	while (*l == '>') {
		++l;		/* step over */
		cont_level++;
	}
#ifdef ENABLE_CONDITIONALS
	if (cont_level == 0 || cont_level > last_cont_level)
		if (file_check_mem(ms, cont_level) == -1)
					    "in_offset `%s' invalid", l);
			l = t;
		}
		if (*l++ != ')' ||
		    ((m->in_op & FILE_OPINDIRECT) && *l++ != ')'))
			if (ms->flags & MAGIC_CHECK)
				file_magwarn(ms,
				    "missing ')' in indirect offset");
		/*
		 * Try it as a keyword type prefixed by "u"; match what
		 * follows the "u".  If that fails, try it as an SUS
		 * integer type.
		 */
		m->type = get_type(type_tbl, l + 1, &l);
		if (m->type == FILE_INVALID) {
			/*
			 */
			m->type = get_standard_integer_type(l, &l);
		}
		// It's unsigned.
		if (m->type != FILE_INVALID)
			m->flag |= UNSIGNED;
	} else {
		/*
		/* Not found - try it as a special keyword. */
		m->type = get_type(special_tbl, l, &l);
	}

	if (m->type == FILE_INVALID) {
		if (ms->flags & MAGIC_CHECK)
			file_magwarn(ms, "type `%s' invalid", l);
		/*if (me->mp) {
			efree(me->mp);
			me->mp = NULL;
		}*/
		return -1;
	}

	/* New-style anding: "0 byte&0x80 =0x80 dynamically linked" */
	m->str_range = 0;
	m->str_flags = m->type == FILE_PSTRING ? PSTRING_1_LE : 0;
	if ((op = get_op(*l)) != -1) {
		if (!IS_LIBMAGIC_STRING(m->type)) {
			uint64_t val;
			++l;
			m->mask_op |= op;
			val = (uint64_t)strtoull(l, &t, 0);
			l = t;
			m->num_mask = file_signextend(ms, m, val);
			eatsize(&l);
		}
		else if (op == FILE_OPDIVIDE) {
			int have_range = 0;
			while (!isspace((unsigned char)*++l)) {
				switch (*l) {
				case '0':  case '1':  case '2':
				case '3':  case '4':  case '5':
				case '6':  case '7':  case '8':
				case '9':
					if (have_range &&
					    (ms->flags & MAGIC_CHECK))
						file_magwarn(ms,
						    "multiple ranges");
					have_range = 1;
					m->str_range = CAST(uint32_t,
					    strtoul(l, &t, 0));
					if (m->str_range == 0)
						file_magwarn(ms,
						    "zero range");
					l = t - 1;
					break;
				case CHAR_COMPACT_WHITESPACE:
					m->str_flags |=
					    STRING_COMPACT_WHITESPACE;
					break;
				case CHAR_COMPACT_OPTIONAL_WHITESPACE:
					m->str_flags |=
					    STRING_COMPACT_OPTIONAL_WHITESPACE;
					break;
				case CHAR_IGNORE_LOWERCASE:
					m->str_flags |= STRING_IGNORE_LOWERCASE;
					break;
				case CHAR_IGNORE_UPPERCASE:
					m->str_flags |= STRING_IGNORE_UPPERCASE;
					break;
				case CHAR_REGEX_OFFSET_START:
					m->str_flags |= REGEX_OFFSET_START;
					break;
				case CHAR_BINTEST:
					m->str_flags |= STRING_BINTEST;
					break;
				case CHAR_TEXTTEST:
					m->str_flags |= STRING_TEXTTEST;
					break;
				case CHAR_TRIM:
					m->str_flags |= STRING_TRIM;
					break;
				case CHAR_PSTRING_1_LE:
					if (m->type != FILE_PSTRING)
						goto bad;
					m->str_flags = (m->str_flags & ~PSTRING_LEN) | PSTRING_1_LE;
					break;
				case CHAR_PSTRING_2_BE:
					if (m->type != FILE_PSTRING)
						goto bad;
					m->str_flags = (m->str_flags & ~PSTRING_LEN) | PSTRING_2_BE;
					break;
				case CHAR_PSTRING_2_LE:
					if (m->type != FILE_PSTRING)
						goto bad;
					m->str_flags = (m->str_flags & ~PSTRING_LEN) | PSTRING_2_LE;
					break;
				case CHAR_PSTRING_4_BE:
					if (m->type != FILE_PSTRING)
						goto bad;
					m->str_flags = (m->str_flags & ~PSTRING_LEN) | PSTRING_4_BE;
					break;
				case CHAR_PSTRING_4_LE:
					if (m->type != FILE_PSTRING)
						goto bad;
					m->str_flags = (m->str_flags & ~PSTRING_LEN) | PSTRING_4_LE;
					break;
				case CHAR_PSTRING_LENGTH_INCLUDES_ITSELF:
					if (m->type != FILE_PSTRING)
						goto bad;
					m->str_flags |= PSTRING_LENGTH_INCLUDES_ITSELF;
					break;
				default:
				bad:
					if (ms->flags & MAGIC_CHECK)
						file_magwarn(ms,
						    "string extension `%c' "
						    "invalid", *l);
					return -1;
				}
				/* allow multiple '/' for readability */
				if (l[1] == '/' &&
				    !isspace((unsigned char)l[2]))
					l++;
			}
			if (string_modifier_check(ms, m) == -1)
				return -1;
		}
		else {
			if (ms->flags & MAGIC_CHECK)
				file_magwarn(ms, "invalid string op: %c", *t);
			return -1;
		}
	}
	/*
	 * We used to set mask to all 1's here, instead let's just not do
	 * anything if mask = 0 (unless you have a better idea)
	 */
	EATAB;

	switch (*l) {
	case '>':
	case '<':
  		m->reln = *l;
		break;
	default:
  		m->reln = '=';	/* the default relation */
		if (*l == 'x' && ((isascii((unsigned char)l[1]) &&
		    isspace((unsigned char)l[1])) || !l[1])) {
			m->reln = *l;
			++l;
		}

	/*
	 * TODO finish this macro and start using it!
	 * #define offsetcheck {if (offset > HOWMANY-1)
	 *	magwarn("offset too big"); }
	 */

	/*
	return -1;
}

/*
 * Parse an Apple CREATOR/TYPE annotation from magic file and put it into
 * magic[index - 1]
 */
private int
parse_apple(struct magic_set *ms, struct magic_entry *me, const char *line)
{
	size_t i;
	const char *l = line;
	struct magic *m = &me->mp[me->cont_count == 0 ? 0 : me->cont_count - 1];

	if (m->apple[0] != '\0') {
		file_magwarn(ms, "Current entry already has a APPLE type "
		    "`%.8s', new type `%s'", m->mimetype, l);
		return -1;
	}

	EATAB;
	for (i = 0; *l && ((isascii((unsigned char)*l) &&
	    isalnum((unsigned char)*l)) || strchr("-+/.", *l)) &&
	    i < sizeof(m->apple); m->apple[i++] = *l++)
		continue;
	if (i == sizeof(m->apple) && *l) {
		/* We don't need to NUL terminate here, printing handles it */
		if (ms->flags & MAGIC_CHECK)
			file_magwarn(ms, "APPLE type `%s' truncated %"
			    SIZE_T_FORMAT "u", line, i);
	}

	if (i > 0)
		return 0;
	else
		return -1;
}

/*
 * parse a MIME annotation line from magic file, put into magic[index - 1]
private int
parse_mime(struct magic_set *ms, struct magic_entry *me, const char *line)
{
	size_t i;
	const char *l = line;
	struct magic *m = &me->mp[me->cont_count == 0 ? 0 : me->cont_count - 1];

	if (m->mimetype[0] != '\0') {
		file_magwarn(ms, "Current entry already has a MIME type `%s',"
		    " new type `%s'", m->mimetype, l);
		return -1;
	}

	EATAB;
	for (i = 0; *l && ((isascii((unsigned char)*l) &&
	    isalnum((unsigned char)*l)) || strchr("-+/.", *l)) &&
	    i < sizeof(m->mimetype); m->mimetype[i++] = *l++)
		continue;
	if (i == sizeof(m->mimetype)) {
		m->mimetype[sizeof(m->mimetype) - 1] = '\0';
		if (ms->flags & MAGIC_CHECK)
			file_magwarn(ms, "MIME type `%s' truncated %"
			    SIZE_T_FORMAT "u", m->mimetype, i);
	} else
		m->mimetype[i] = '\0';

	if (i > 0)
		return 0;
	else
		return -1;
}

private int
check_format_type(const char *ptr, int type)
{
	int quad = 0;
	if (*ptr == '\0') {
		/* Missing format string; bad */
		return -1;
	}

	switch (type) {
	case FILE_FMT_QUAD:
		quad = 1;
		/*FALLTHROUGH*/
	case FILE_FMT_NUM:
		if (*ptr == '-')
			ptr++;
		if (*ptr == '.')
			ptr++;
			if (*ptr++ != 'l')
				return -1;
		}

		switch (*ptr++) {
		case 'l':
			switch (*ptr++) {
			case 'i':
			case 'd':
			case 'o':
			case 'x':
			case 'X':
				return 0;
			default:
				return -1;
			}

		case 'h':
			switch (*ptr++) {
			case 'h':
				switch (*ptr++) {
				case 'i':
				case 'd':
				case 'u':
				default:
					return -1;
				}
			case 'd':
				return 0;
			default:
				return -1;
			}

		case 'i':
		case 'c':
		case 'd':
		case 'u':
		case 'o':
		case 'x':
		case 'X':
			return 0;

		default:
			return -1;
		}

	case FILE_FMT_FLOAT:
	case FILE_FMT_DOUBLE:
		if (*ptr == '-')
			ptr++;
		if (*ptr == '.')
			ptr++;
		while (isdigit((unsigned char)*ptr)) ptr++;

		switch (*ptr++) {
		case 'e':
		case 'E':
		case 'f':
		case 'g':
		case 'G':
			return 0;

		default:
			return -1;
		}


	case FILE_FMT_STR:
		if (*ptr == '-')
			ptr++;
			while (isdigit((unsigned char )*ptr))
				ptr++;
		}

		switch (*ptr++) {
		case 's':
			return 0;
		default:
			return -1;
		}

	default:
		/* internal error */
		abort();
	}
	/*NOTREACHED*/
	return -1;
}

/*
 * Check that the optional printf format in description matches
 * the type of the magic.
 */

	if (m->type >= file_nformats) {
		file_magwarn(ms, "Internal error inconsistency between "
		    "m->type and format strings");
		return -1;
	}
	if (file_formats[m->type] == FILE_FMT_NONE) {
		file_magwarn(ms, "No format string for `%s' with description "
	}

	ptr++;
	if (check_format_type(ptr, file_formats[m->type]) == -1) {
		/*
		 * TODO: this error message is unhelpful if the format
		 * string is not one character long
		 */
		    file_names[m->type], m->desc);
		return -1;
	}

	for (; *ptr; ptr++) {
		if (*ptr == '%') {
			file_magwarn(ms,
			    "Too many format strings (should have at most one) "
	return 0;
}

/*
 * Read a numeric value from a pointer, into the value union of a magic
 * pointer, according to the magic type.  Update the string pointer to point
 * just after the number read.  Return 0 for success, non-zero for failure.
 */
private int
getvalue(struct magic_set *ms, struct magic *m, const char **p, int action)
				    m->value.s);
			return -1;
		}
		return 0;
	case FILE_FLOAT:
	case FILE_BEFLOAT:
	case FILE_LEFLOAT:
			default:
				if (warn) {
					if (isprint((unsigned char)c)) {
						/* Allow escaping of
						 * ``relations'' */
						if (strchr("<>&^=!", c) == NULL
						    && (m->type != FILE_REGEX ||
						    strchr("[]().*?^$|{}", c)
{
	const char *l = *p;

	if (LOWCASE(*l) == 'u')
		l++;

	switch (LOWCASE(*l)) {
	case 'l':    /* long */
	*p = l;
}

/*
 * handle a compiled file.
 */

		file_error(ms, errno, "cannot stat `%s'", dbname);
		goto error;
	}

	if (st.sb.st_size < 8) {
		file_error(ms, 0, "file `%s' is too small", dbname);
		goto error;
	}

	map->len = (size_t)st.sb.st_size;
		    dbname, entries, nentries + 1);
		goto error;
	}

	if (needsbyteswap)
		for (i = 0; i < MAGIC_SETS; i++)
			byteswap(map->magic[i], map->nmagic[i]);

	return NULL;
}

private const uint32_t ar[] = {
    MAGICNO, VERSIONNO
};

/*
 * handle an mmaped file.
 */
	char *dbname;
	int rv = -1;
	uint32_t i;
	php_stream *stream;


	dbname = mkdbname(ms, fn, 0);
		file_error(ms, errno, "cannot open `%s'", dbname);
		goto out;
	}

	if (write(fd, ar, sizeof(ar)) != (ssize_t)sizeof(ar)) {
		file_error(ms, errno, "error writing `%s'", dbname);
		goto out;
	}

	if (php_stream_write(stream, (const char *)map->nmagic, nm) != (ssize_t)nm) {
		file_error(ms, errno, "error writing `%s'", dbname);
		goto out;
	}

	assert(nm + sizeof(ar) < m);

	if (php_stream_seek(stream,(zend_off_t)sizeof(struct magic), SEEK_SET) != sizeof(struct magic)) {
		file_error(ms, errno, "error seeking `%s'", dbname);
		goto out;
	}

	for (i = 0; i < MAGIC_SETS; i++) {
		len = m * map->nmagic[i];
		if (php_stream_write(stream, (const char *)map->magic[i], len) != (ssize_t)len) {
			file_error(ms, errno, "error writing `%s'", dbname);
	if (stream) {
		php_stream_close(stream);
	}

	rv = 0;
out:
	efree(dbname);
	return rv;
swap2(uint16_t sv)
{
	uint16_t rv;
	uint8_t *s = (uint8_t *)(void *)&sv;
	uint8_t *d = (uint8_t *)(void *)&rv;
	d[0] = s[1];
	d[1] = s[0];
	return rv;
}
swap4(uint32_t sv)
{
	uint32_t rv;
	uint8_t *s = (uint8_t *)(void *)&sv;
	uint8_t *d = (uint8_t *)(void *)&rv;
	d[0] = s[3];
	d[1] = s[2];
	d[2] = s[1];
	d[3] = s[0];
swap8(uint64_t sv)
{
	uint64_t rv;
	uint8_t *s = (uint8_t *)(void *)&sv;
	uint8_t *d = (uint8_t *)(void *)&rv;
#if 0
	d[0] = s[3];
	d[1] = s[2];
	d[2] = s[1];
	}
}

protected size_t
file_pstring_length_size(const struct magic *m)
{
	switch (m->str_flags & PSTRING_LEN) {
	case PSTRING_1_LE: