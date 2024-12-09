#include "file.h"

#ifndef	lint
FILE_RCSID("@(#)$File: apprentice.c,v 1.342 2023/07/17 14:38:35 christos Exp $")
#endif	/* lint */

#include "magic.h"
#include <stdlib.h>
const char *file_names[FILE_NAMES_SIZE];
const size_t file_nnames = FILE_NAMES_SIZE;

file_private int getvalue(struct magic_set *ms, struct magic *, const char **, int);
file_private int hextoint(int);
file_private const char *getstr(struct magic_set *, struct magic *, const char *,
    int);
file_private int parse(struct magic_set *, struct magic_entry *, const char *,
    size_t, int);
file_private void eatsize(const char **);
file_private int apprentice_1(struct magic_set *, const char *, int);
file_private ssize_t apprentice_magic_strength_1(const struct magic *);
file_private int apprentice_sort(const void *, const void *);
file_private void apprentice_list(struct mlist *, int );
file_private struct magic_map *apprentice_load(struct magic_set *,
    const char *, int);
file_private struct mlist *mlist_alloc(void);
file_private void mlist_free_all(struct magic_set *);
file_private void mlist_free(struct mlist *);
file_private void byteswap(struct magic *, uint32_t);
file_private void bs1(struct magic *);

#if defined(HAVE_BYTESWAP_H)
#define swap2(x)	bswap_16(x)
#define swap4(x)	bswap_32(x)
#define swap4(x)	bswap32(x)
#define swap8(x)	bswap64(x)
#else
file_private uint16_t swap2(uint16_t);
file_private uint32_t swap4(uint32_t);
file_private uint64_t swap8(uint64_t);
#endif

file_private char *mkdbname(struct magic_set *, const char *, int);
file_private struct magic_map *apprentice_map(struct magic_set *, const char *);
file_private void apprentice_unmap(struct magic_map *);
file_private int apprentice_compile(struct magic_set *, struct magic_map *,
    const char *);
file_private int check_format_type(const char *, int, const char **);
file_private int check_format(struct magic_set *, struct magic *);
file_private int get_op(char);
file_private int parse_mime(struct magic_set *, struct magic_entry *, const char *,
    size_t);
file_private int parse_strength(struct magic_set *, struct magic_entry *,
    const char *, size_t);
file_private int parse_apple(struct magic_set *, struct magic_entry *, const char *,
    size_t);
file_private int parse_ext(struct magic_set *, struct magic_entry *, const char *,
    size_t);


file_private size_t magicsize = sizeof(struct magic);

file_private const char usg_hdr[] = "cont\toffset\ttype\topcode\tmask\tvalue\tdesc";

file_private struct {
	const char *name;
	size_t len;
	int (*fun)(struct magic_set *, struct magic_entry *, const char *,
	    size_t);
# undef XX
# undef XX_NULL

file_private int
get_type(const struct type_tbl_s *tbl, const char *l, const char **t)
{
	const struct type_tbl_s *p;

	return p->type;
}

file_private off_t
maxoff_t(void) {
	if (/*CONSTCOND*/sizeof(off_t) == sizeof(int))
		return CAST(off_t, INT_MAX);
	if (/*CONSTCOND*/sizeof(off_t) == sizeof(long))
	return 0x7fffffff;
}

file_private int
get_standard_integer_type(const char *l, const char **t)
{
	int type;

	return type;
}

file_private void
init_file_tables(void)
{
	static int done = 0;
	const struct type_tbl_s *p;
	assert(p - type_tbl == FILE_NAMES_SIZE);
}

file_private int
add_mlist(struct mlist *mlp, struct magic_map *map, size_t idx)
{
	struct mlist *ml;

/*
 * Handle one file or directory.
 */
file_private int
apprentice_1(struct magic_set *ms, const char *fn, int action)
{
	struct magic_map *map;
#ifndef COMPILE_ONLY
	size_t i;
#endif

	if (magicsize != FILE_MAGICSIZE) {
	map = apprentice_map(ms, fn);
	if (map == NULL) {
		if (ms->flags & MAGIC_CHECK)
			file_magwarn(NULL, "using regular magic file `%s'", fn);
		map = apprentice_load(ms, fn, action);
		if (map == NULL)
			return -1;
	}
				apprentice_unmap(map);
			else
				mlist_free_all(ms);
			file_oomem(ms, sizeof(*ms->mlist[0]));
			return -1;
		}
	}

#endif /* COMPILE_ONLY */
}

file_protected void
file_ms_free(struct magic_set *ms)
{
	size_t i;
	if (ms == NULL)
	efree(ms);
}

file_protected struct magic_set *
file_ms_alloc(int flags)
{
	struct magic_set *ms;
	size_t i, len;

	if ((ms = CAST(struct magic_set *, ecalloc(CAST(size_t, 1u),
	    sizeof(*ms)))) == NULL)
		return NULL;

	if (magic_setflags(ms, flags) == -1) {
		errno = EINVAL;
	ms->indir_max = FILE_INDIR_MAX;
	ms->name_max = FILE_NAME_MAX;
	ms->elf_shnum_max = FILE_ELF_SHNUM_MAX;
	ms->elf_shsize_max = FILE_ELF_SHSIZE_MAX;
	ms->elf_phnum_max = FILE_ELF_PHNUM_MAX;
	ms->elf_notes_max = FILE_ELF_NOTES_MAX;
	ms->regex_max = FILE_REGEX_MAX;
	ms->bytes_max = FILE_BYTES_MAX;
#endif
	return ms;
free:
	efree(ms);
	return NULL;
}

file_private void
apprentice_unmap(struct magic_map *map)
{
	if (map == NULL)
		return;
	efree(map);
}

file_private struct mlist *
mlist_alloc(void)
{
	struct mlist *mlist;
	if ((mlist = CAST(struct mlist *, ecalloc(1, sizeof(*mlist)))) == NULL) {
	return mlist;
}

file_private void
mlist_free_all(struct magic_set *ms)
{
	size_t i;

	}
}

file_private void
mlist_free_one(struct mlist *ml)
{
	if (ml->map)
		apprentice_unmap(CAST(struct magic_map *, ml->map));
	efree(ml);
}

file_private void
mlist_free(struct mlist *mlist)
{
	struct mlist *ml, *next;

}

/* const char *fn: list of magic files and directories */
file_protected int
file_apprentice(struct magic_set *ms, const char *fn, int action)
{
	char *p, *mfn;
	int fileerr, errs = -1;
	for (i = 0; i < MAGIC_SETS; i++) {
		mlist_free(ms->mlist[i]);
		if ((ms->mlist[i] = mlist_alloc()) == NULL) {
			file_oomem(ms, sizeof(*ms->mlist[0]));
			for (j = 0; j < i; j++) {
				mlist_free(ms->mlist[j]);
				ms->mlist[j] = NULL;
			}
 *	- regular characters or escaped magic characters count 1
 *	- 0 length expressions count as one
 */
file_private size_t
nonmagic(const char *str)
{
	const char *p;
	size_t rv = 0;
}


file_private size_t
typesize(int type)
{
	switch (type) {
	case FILE_BYTE:
/*
 * Get weight of this magic entry, for sorting purposes.
 */
file_private ssize_t
apprentice_magic_strength_1(const struct magic *m)
{
#define MULT 10U
	size_t ts, v;
	switch (m->type) {
	case FILE_DEFAULT:	/* make sure this sorts last */
		if (m->factor_op != FILE_FACTOR_OP_NONE) {
			file_magwarn(NULL, "Usupported factor_op in default %d",
			    m->factor_op);
		}
		return 0;

	case FILE_BYTE:


/*ARGSUSED*/
file_protected size_t
file_magic_strength(const struct magic *m,
    size_t nmagic __attribute__((__unused__)))
{
	ssize_t val = apprentice_magic_strength_1(m);

/*
 * Sort callback for sorting entries by "strength" (basically length)
 */
file_private int
apprentice_sort(const void *a, const void *b)
{
	const struct magic_entry *ma = CAST(const struct magic_entry *, a);
	const struct magic_entry *mb = CAST(const struct magic_entry *, b);
	size_t sa = file_magic_strength(ma->mp, ma->cont_count);
	size_t sb = file_magic_strength(mb->mp, mb->cont_count);
	if (sa == sb)
		return 0;
	else if (sa > sb)
		return -1;
/*
 * Shows sorted patterns list in the order which is used for the matching
 */
file_private void
apprentice_list(struct mlist *mlist, int mode)
{
	uint32_t magindex, descindex, mimeindex, lineindex;
	struct mlist *ml;
			 * description/mimetype.
			 */
			lineindex = descindex = mimeindex = magindex;
			for (; magindex + 1 < ml->nmagic &&
			   ml->magic[magindex + 1].cont_level != 0;
			   magindex++) {
				uint32_t mi = magindex + 1;
				if (*ml->magic[descindex].desc == '\0'
				    && *ml->magic[mi].desc)
					descindex = mi;
				if (*ml->magic[mimeindex].mimetype == '\0'
				    && *ml->magic[mi].mimetype)
					mimeindex = mi;
			}

			printf("Strength = %3" SIZE_T_FORMAT "u@%u: %s [%s]\n",
			    file_magic_strength(m, ml->nmagic - magindex),
			    ml->magic[lineindex].lineno,
			    ml->magic[descindex].desc,
			    ml->magic[mimeindex].mimetype);
		}
	}
}

file_private void
set_test_type(struct magic *mstart, struct magic *m)
{
	switch (m->type) {
	case FILE_BYTE:
	}
}

file_private int
addentry(struct magic_set *ms, struct magic_entry *me,
   struct magic_entry_set *mset)
{
	size_t i = me->mp->type == FILE_NAME ? 1 : 0;
/*
 * Load and parse one file.
 */
file_private void
load_1(struct magic_set *ms, int action, const char *fn, int *errs,
   struct magic_entry_set *mset)
{
	char buffer[BUFSIZ + 1];
 * parse a file or directory of files
 * const char *fn: name of magic file or directory
 */
file_private int
cmpstrp(const void *p1, const void *p2)
{
        return strcmp(*RCAST(char *const *, p1), *RCAST(char *const *, p2));
}


file_private uint32_t
set_text_binary(struct magic_set *ms, struct magic_entry *me, uint32_t nme,
    uint32_t starttest)
{
	static const char text[] = "text";
	return i;
}

file_private void
set_last_default(struct magic_set *ms, struct magic_entry *me, uint32_t nme)
{
	uint32_t i;
	for (i = 0; i < nme; i++) {
	}
}

file_private int
coalesce_entries(struct magic_set *ms, struct magic_entry *me, uint32_t nme,
    struct magic **ma, uint32_t *nma)
{
	uint32_t i, mentrycount = 0;
	return 0;
}

file_private void
magic_entry_free(struct magic_entry *me, uint32_t nme)
{
	uint32_t i;
	if (me == NULL)
	efree(me);
}

file_private struct magic_map *
apprentice_load(struct magic_set *ms, const char *fn, int action)
{
	int errs = 0;
	uint32_t i, j;
			i = set_text_binary(ms, mset[j].me, mset[j].count, i);
		}
		if (mset[j].me)
			qsort(mset[j].me, mset[j].count, sizeof(*mset[0].me),
			    apprentice_sort);

		/*
		 * Make sure that any level 0 "default" line is last
/*
 * extend the sign bit if the comparison is to be signed
 */
file_protected uint64_t
file_signextend(struct magic_set *ms, struct magic *m, uint64_t v)
{
	if (!(m->flag & UNSIGNED)) {
		switch(m->type) {
	return v;
}

file_private int
string_modifier_check(struct magic_set *ms, struct magic *m)
{
	if ((ms->flags & MAGIC_CHECK) == 0)
		return 0;
	return 0;
}

file_private int
get_op(char c)
{
	switch (c) {
	case '&':
}

#ifdef ENABLE_CONDITIONALS
file_private int
get_cond(const char *l, const char **t)
{
	static const struct cond_tbl_s {
		char name[8];
	return p->cond;
}

file_private int
check_cond(struct magic_set *ms, int cond, uint32_t cont_level)
{
	int last_cond;
	last_cond = ms->c.li[cont_level].last_cond;
}
#endif /* ENABLE_CONDITIONALS */

file_private int
parse_indirect_modifier(struct magic_set *ms, struct magic *m, const char **lp)
{
	const char *l = *lp;

	return 0;
}

file_private void
parse_op_modifier(struct magic_set *ms, struct magic *m, const char **lp,
    int op)
{
	const char *l = *lp;
	*lp = l;
}

file_private int
parse_string_modifier(struct magic_set *ms, struct magic *m, const char **lp)
{
	const char *l = *lp;
	char *t;
/*
 * parse one line from magic file, put into magic[index++] if valid
 */
file_private int
parse(struct magic_set *ms, struct magic_entry *me, const char *line,
    size_t lineno, int action)
{
#ifdef ENABLE_CONDITIONALS
 * if valid
 */
/*ARGSUSED*/
file_private int
parse_strength(struct magic_set *ms, struct magic_entry *me, const char *line,
    size_t len __attribute__((__unused__)))
{
	const char *l = line;
	char *el;
	unsigned long factor;
	char sbuf[512];
	struct magic *m = &me->mp[0];

	if (m->factor_op != FILE_FACTOR_OP_NONE) {
		file_magwarn(ms,
	}
	if (m->type == FILE_NAME) {
		file_magwarn(ms, "%s: Strength setting is not supported in "
		    "\"name\" magic entries",
		    file_printable(ms, sbuf, sizeof(sbuf), m->value.s,
		    sizeof(m->value.s)));
		return -1;
	}
	EATAB;
	switch (*l) {
	case FILE_FACTOR_OP_NONE:
		break;
	case FILE_FACTOR_OP_PLUS:
	case FILE_FACTOR_OP_MINUS:
	case FILE_FACTOR_OP_TIMES:
	case FILE_FACTOR_OP_DIV:
	return -1;
}

file_private int
goodchar(unsigned char x, const char *extra)
{
	return (isascii(x) && isalnum(x)) || strchr(extra, x);
}

file_private int
parse_extra(struct magic_set *ms, struct magic_entry *me, const char *line,
    size_t llen, zend_off_t off, size_t len, const char *name, const char *extra,
    int nt)
{
	size_t i;
	const char *l = line;
	struct magic *m = &me->mp[me->cont_count == 0 ? 0 : me->cont_count - 1];
 * Parse an Apple CREATOR/TYPE annotation from magic file and put it into
 * magic[index - 1]
 */
file_private int
parse_apple(struct magic_set *ms, struct magic_entry *me, const char *line,
    size_t len)
{
	return parse_extra(ms, me, line, len,
/*
 * Parse a comma-separated list of extensions
 */
file_private int
parse_ext(struct magic_set *ms, struct magic_entry *me, const char *line,
    size_t len)
{
	return parse_extra(ms, me, line, len,
	    CAST(off_t, offsetof(struct magic, ext)),
	    sizeof(me->mp[0].ext), "EXTENSION", ",!+-/@?_$&~", 0);
	    /* & for b&w */
	    /* ~ for journal~ */
}

/*
 * parse a MIME annotation line from magic file, put into magic[index - 1]
 * if valid
 */
file_private int
parse_mime(struct magic_set *ms, struct magic_entry *me, const char *line,
    size_t len)
{
	return parse_extra(ms, me, line, len,
	    sizeof(me->mp[0].mimetype), "MIME", "+-/.$?:{}", 1);
}

file_private int
check_format_type(const char *ptr, int type, const char **estr)
{
	int quad = 0, h;
	size_t len, cnt;
	}
invalid:
	*estr = "not valid";
	return -1;
toolong:
	*estr = "too long";
	return -1;
}
 * Check that the optional printf format in description matches
 * the type of the magic.
 */
file_private int
check_format(struct magic_set *ms, struct magic *m)
{
	char *ptr;
	const char *estr;
 * pointer, according to the magic type.  Update the string pointer to point
 * just after the number read.  Return 0 for success, non-zero for failure.
 */
file_private int
getvalue(struct magic_set *ms, struct magic *m, const char **p, int action)
{
	char *ep;
	uint64_t ull;
	int y;

	switch (m->type) {
	case FILE_BESTRING16:
	case FILE_LESTRING16:
		m->value.q = file_signextend(ms, m, ull);
		if (*p == ep) {
			file_magwarn(ms, "Unparsable number `%s'", *p);
			return -1;
		} else {
			size_t ts = typesize(m->type);
			uint64_t x;
			const char *q;
				file_magwarn(ms,
				    "Expected numeric type got `%s'",
				    type_tbl[m->type].name);
				return -1;
			}
			for (q = *p; isspace(CAST(unsigned char, *q)); q++)
				continue;
			if (*q == '-' && ull != UINT64_MAX)
				ull = -CAST(int64_t, ull);
			switch (ts) {
			case 1:
				x = CAST(uint64_t, ull & ~0xffULL);
				y = (x & ~0xffULL) != ~0xffULL;
				break;
			case 2:
				x = CAST(uint64_t, ull & ~0xffffULL);
				y = (x & ~0xffffULL) != ~0xffffULL;
				break;
			case 4:
				x = CAST(uint64_t, ull & ~0xffffffffULL);
				y = (x & ~0xffffffffULL) != ~0xffffffffULL;
				break;
			case 8:
				x = 0;
				y = 0;
				break;
			default:
				fprintf(stderr, "Bad width %zu", ts);
				abort();
			}
			if (x && y) {
				file_magwarn(ms, "Overflow for numeric"
				    " type `%s' value %#" PRIx64,
				    type_tbl[m->type].name, ull);
				return -1;
			}
		}
		if (errno == 0) {
			*p = ep;
 * Copy the converted version to "m->value.s", and the length in m->vallen.
 * Return updated scan pointer as function result. Warn if set.
 */
file_private const char *
getstr(struct magic_set *ms, struct magic *m, const char *s, int warn)
{
	const char *origs = s;
	char	*p = m->value.s;


/* Single hex char to int; -1 if not a hex char. */
file_private int
hextoint(int c)
{
	if (!isascii(CAST(unsigned char, c)))
		return -1;
/*
 * Print a string containing C character escapes.
 */
file_protected void
file_showstr(FILE *fp, const char *s, size_t len)
{
	char	c;

/*
 * eatsize(): Eat the size spec from a number [eg. 10UL]
 */
file_private void
eatsize(const char **p)
{
	const char *l = *p;

 * handle a compiled file.
 */

file_private struct magic_map *
apprentice_map(struct magic_set *ms, const char *fn)
{
	uint32_t *ptr;
	uint32_t version, entries = 0, nentries;
/*
 * handle an mmaped file.
 */
file_private int
apprentice_compile(struct magic_set *ms, struct magic_map *map, const char *fn)
{
	static const size_t nm = sizeof(*map->nmagic) * MAGIC_SETS;
	static const size_t m = sizeof(**map->magic);
	return rv;
}

file_private const char ext[] = ".mgc";
/*
 * make a dbname
 */
file_private char *
mkdbname(struct magic_set *ms, const char *fn, int strip)
{
	const char *p, *q;
	char *buf;
/*
 * Byteswap an mmap'ed file if needed
 */
file_private void
byteswap(struct magic *magic, uint32_t nmagic)
{
	uint32_t i;
	for (i = 0; i < nmagic; i++)
/*
 * swap a short
 */
file_private uint16_t
swap2(uint16_t sv)
{
	uint16_t rv;
	uint8_t *s = RCAST(uint8_t *, RCAST(void *, &sv));
/*
 * swap an int
 */
file_private uint32_t
swap4(uint32_t sv)
{
	uint32_t rv;
	uint8_t *s = RCAST(uint8_t *, RCAST(void *, &sv));
/*
 * swap a quad
 */
file_private uint64_t
swap8(uint64_t sv)
{
	uint64_t rv;
	uint8_t *s = RCAST(uint8_t *, RCAST(void *, &sv));
}
#endif

file_protected uintmax_t 
file_varint2uintmax_t(const unsigned char *us, int t, size_t *l)
{
        uintmax_t x = 0;
        const unsigned char *c;
/*
 * byteswap a single magic entry
 */
file_private void
bs1(struct magic *m)
{
	m->cont_level = swap2(m->cont_level);
	m->offset = swap4(CAST(uint32_t, m->offset));
	}
}

file_protected size_t
file_pstring_length_size(struct magic_set *ms, const struct magic *m)
{
	switch (m->str_flags & PSTRING_LEN) {
	case PSTRING_1_LE:
		return FILE_BADSIZE;
	}
}
file_protected size_t
file_pstring_get_length(struct magic_set *ms, const struct magic *m,
    const char *ss)
{
	size_t len = 0;
	return len;
}

file_protected int
file_magicfind(struct magic_set *ms, const char *name, struct mlist *v)
{
	uint32_t i, j;
	struct mlist *mlist, *ml;