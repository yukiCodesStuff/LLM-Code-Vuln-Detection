#include "file.h"

#ifndef	lint
FILE_RCSID("@(#)$File: softmagic.c,v 1.174 2014/02/12 23:20:53 christos Exp $")
#endif	/* lint */

#include "magic.h"
#ifdef HAVE_FMTCHECK
#include <stdio.h>
#define F(a, b) fmtcheck((a), (b))
#else
#define F(a, b) (a)
#endif
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>


private int match(struct magic_set *, struct magic *, uint32_t,
    const unsigned char *, size_t, size_t, int, int, int, int, int *, int *,
    int *);
private int mget(struct magic_set *, const unsigned char *,
    struct magic *, size_t, size_t, unsigned int, int, int, int, int, int *,
    int *, int *);
private int magiccheck(struct magic_set *, struct magic *);
private int32_t mprint(struct magic_set *, struct magic *);
private int32_t moffset(struct magic_set *, struct magic *);
private void mdebug(uint32_t, const char *, size_t);
/*ARGSUSED1*/		/* nbytes passed for regularity, maybe need later */
protected int
file_softmagic(struct magic_set *ms, const unsigned char *buf, size_t nbytes,
    size_t level, int mode, int text)
{
	struct mlist *ml;
	int rv, printed_something = 0, need_separator = 0;
	for (ml = ms->mlist[0]->next; ml != ms->mlist[0]; ml = ml->next)
		if ((rv = match(ms, ml->magic, ml->nmagic, buf, nbytes, 0, mode,
		    text, 0, level, &printed_something, &need_separator,
		    NULL)) != 0)
			return rv;

	return 0;
}

/*
 * Go through the whole list, stopping if you find a match.  Process all
 * the continuations of that match before returning.
 *
private int
match(struct magic_set *ms, struct magic *magic, uint32_t nmagic,
    const unsigned char *s, size_t nbytes, size_t offset, int mode, int text,
    int flip, int recursion_level, int *printed_something, int *need_separator,
    int *returnval)
{
	uint32_t magindex = 0;
	unsigned int cont_level = 0;
	int returnvalv = 0, e; /* if a match is found it is set to 1*/

		/* if main entry matches, print it... */
		switch (mget(ms, s, m, nbytes, offset, cont_level, mode, text,
		    flip, recursion_level + 1, printed_something,
		    need_separator, returnval)) {
		case -1:
			return -1;
		case 0:
			flush = m->reln != '!';
		if (file_check_mem(ms, ++cont_level) == -1)
			return -1;

		while (magindex + 1 < nmagic && magic[magindex+1].cont_level != 0 &&
		    ++magindex) {
			m = &magic[magindex];
			ms->line = m->lineno; /* for messages */

			if (cont_level < m->cont_level)
				continue;
			}
#endif
			switch (mget(ms, s, m, nbytes, offset, cont_level, mode,
			    text, flip, recursion_level + 1, printed_something,
			    need_separator, returnval)) {
			case -1:
				return -1;
			case 0:
				if (m->reln != '!')
		if ((ms->flags & MAGIC_CONTINUE) == 0 && *printed_something) {
			return *returnval; /* don't keep searching */
		}
	}
	return *returnval;  /* This is hit if -k is set or there is no match */
}

	float vf;
	double vd;
	int64_t t = 0;
 	char buf[128], tbuf[26];
	union VALUETYPE *p = &ms->ms_value;

  	switch (m->type) {
  	case FILE_BYTE:
		case -1:
			return -1;
		case 1:
			(void)snprintf(buf, sizeof(buf), "%c",
			    (unsigned char)v);
			if (file_printf(ms, F(m->desc, "%s"), buf) == -1)
				return -1;
			break;
		default:
			if (file_printf(ms, F(m->desc, "%c"),
			    (unsigned char) v) == -1)
				return -1;
			break;
		}
		case -1:
			return -1;
		case 1:
			(void)snprintf(buf, sizeof(buf), "%hu",
			    (unsigned short)v);
			if (file_printf(ms, F(m->desc, "%s"), buf) == -1)
				return -1;
			break;
		default:
			if (file_printf(ms, F(m->desc, "%hu"),
			    (unsigned short) v) == -1)
				return -1;
			break;
		}
		case -1:
			return -1;
		case 1:
			(void)snprintf(buf, sizeof(buf), "%u", (uint32_t)v);
			if (file_printf(ms, F(m->desc, "%s"), buf) == -1)
				return -1;
			break;
		default:
			if (file_printf(ms, F(m->desc, "%u"),
			    (uint32_t) v) == -1)
				return -1;
			break;
		}
		t = ms->offset + sizeof(int32_t);
		case -1:
			return -1;
		case 1:
			(void)snprintf(buf, sizeof(buf), "%llu",
			    (unsigned long long)v);
			if (file_printf(ms, F(m->desc, "%s"), buf) == -1)
				return -1;
			break;
		default:
			if (file_printf(ms, F(m->desc, "%llu"),
			    (unsigned long long) v) == -1)
				return -1;
			break;
		}
  	case FILE_BESTRING16:
  	case FILE_LESTRING16:
		if (m->reln == '=' || m->reln == '!') {
			if (file_printf(ms, F(m->desc, "%s"), m->value.s) == -1)
				return -1;
			t = ms->offset + m->vallen;
		}
		else {
			t = ms->offset + strlen(str);

			if (*m->value.s == '\0')
				str[strcspn(str, "\n")] = '\0';

			if (m->str_flags & STRING_TRIM) {
				char *last;
				while (isspace((unsigned char)*str))
				*++last = '\0';
			}

			if (file_printf(ms, F(m->desc, "%s"), str) == -1)
				return -1;

			if (m->type == FILE_PSTRING)
				t += file_pstring_length_size(m);
	case FILE_BEDATE:
	case FILE_LEDATE:
	case FILE_MEDATE:
		if (file_printf(ms, F(m->desc, "%s"),
		    file_fmttime(p->l, FILE_T_LOCAL,
		    tbuf)) == -1)
			return -1;
		t = ms->offset + sizeof(uint32_t);
		break;

	case FILE_BELDATE:
	case FILE_LELDATE:
	case FILE_MELDATE:
		if (file_printf(ms, F(m->desc, "%s"),
		    file_fmttime(p->l, 0, tbuf)) == -1)
			return -1;
		t = ms->offset + sizeof(uint32_t);
		break;

	case FILE_QDATE:
	case FILE_BEQDATE:
	case FILE_LEQDATE:
		if (file_printf(ms, F(m->desc, "%s"),
		    file_fmttime(p->q, FILE_T_LOCAL, tbuf)) == -1)
			return -1;
		t = ms->offset + sizeof(uint64_t);
		break;

	case FILE_QLDATE:
	case FILE_BEQLDATE:
	case FILE_LEQLDATE:
		if (file_printf(ms, F(m->desc, "%s"),
		    file_fmttime(p->q, 0, tbuf)) == -1)
			return -1;
		t = ms->offset + sizeof(uint64_t);
		break;

	case FILE_QWDATE:
	case FILE_BEQWDATE:
	case FILE_LEQWDATE:
		if (file_printf(ms, F(m->desc, "%s"),
		    file_fmttime(p->q, FILE_T_WINDOWS, tbuf)) == -1)
			return -1;
		t = ms->offset + sizeof(uint64_t);
		break;

  	case FILE_FLOAT:
  	case FILE_BEFLOAT:
  	case FILE_LEFLOAT:
		vf = p->f;
		switch (check_fmt(ms, m)) {
		case -1:
			return -1;
		case 1:
			(void)snprintf(buf, sizeof(buf), "%g", vf);
			if (file_printf(ms, F(m->desc, "%s"), buf) == -1)
				return -1;
			break;
		default:
			if (file_printf(ms, F(m->desc, "%g"), vf) == -1)
				return -1;
			break;
		}
		t = ms->offset + sizeof(float);
  		break;

  	case FILE_DOUBLE:
  	case FILE_BEDOUBLE:
  	case FILE_LEDOUBLE:
		vd = p->d;
		switch (check_fmt(ms, m)) {
		case -1:
			return -1;
		case 1:
			(void)snprintf(buf, sizeof(buf), "%g", vd);
			if (file_printf(ms, F(m->desc, "%s"), buf) == -1)
				return -1;
			break;
		default:
			if (file_printf(ms, F(m->desc, "%g"), vd) == -1)
				return -1;
			break;
		}
		t = ms->offset + sizeof(double);
  		break;

	case FILE_REGEX: {
		char *cp;
		int rval;

			file_oomem(ms, ms->search.rm_len);
			return -1;
		}
		rval = file_printf(ms, F(m->desc, "%s"), cp);
		efree(cp);

		if (rval == -1)
			return -1;
		break;
	}

	case FILE_SEARCH:
	  	if (file_printf(ms, F(m->desc, "%s"), m->value.s) == -1)
			return -1;
		if ((m->str_flags & REGEX_OFFSET_START))
			t = ms->search.offset;
		else
			t = ms->search.offset + m->vallen;
		break;

	case FILE_DEFAULT:
	case FILE_CLEAR:
	  	if (file_printf(ms, "%s", m->desc) == -1)
			return -1;
			uint32_t t;

			if (*m->value.s == '\0')
				p->s[strcspn(p->s, "\n")] = '\0';
			t = CAST(uint32_t, (ms->offset + strlen(p->s)));
			if (m->type == FILE_PSTRING)
				t += (uint32_t)file_pstring_length_size(m);
			return t;
mconvert(struct magic_set *ms, struct magic *m, int flip)
{
	union VALUETYPE *p = &ms->ms_value;

	switch (cvt_flip(m->type, flip)) {
	case FILE_BYTE:
		cvt_8(p, m);
		return 1;
	case FILE_SHORT:
			 * string by p->s, so we need to deduct sz.
			 * Because we can use one of the bytes of the length
			 * after we shifted as NUL termination.
			 */
			len = sz;
		}
		while (len--)
			*ptr1++ = *ptr2++;
private void
mdebug(uint32_t offset, const char *str, size_t len)
{
	(void) fprintf(stderr, "mget/%zu @%d: ", len, offset);
	file_showstr(stderr, str, len);
	(void) fputc('\n', stderr);
	(void) fputc('\n', stderr);
}
			const char *last;	/* end of search region */
			const char *buf;	/* start of search region */
			const char *end;
			size_t lines, linecnt, bytecnt;

			linecnt = m->str_range;
			bytecnt = linecnt * 80;

			if (bytecnt == 0) {
				bytecnt = 8192;
			}
			if (bytecnt > nbytes) {
				bytecnt = nbytes;
			}
			if (s == NULL) {
				ms->search.s_len = 0;
				ms->search.s = NULL;
				return 0;
			}
			buf = RCAST(const char *, s) + offset;
			end = last = RCAST(const char *, s) + bytecnt;
			/* mget() guarantees buf <= last */
			for (lines = linecnt, b = buf; lines && b < end &&
private int
mget(struct magic_set *ms, const unsigned char *s, struct magic *m,
    size_t nbytes, size_t o, unsigned int cont_level, int mode, int text,
    int flip, int recursion_level, int *printed_something,
    int *need_separator, int *returnval)
{
	uint32_t soffset, offset = ms->offset;
	int rv, oneed_separator, in_type;
	char *sbuf, *rbuf;
	union VALUETYPE *p = &ms->ms_value;
	struct mlist ml;

	if (recursion_level >= 20) {
		file_error(ms, 0, "recursion nesting exceeded");
		return -1;
	}

	if (mcopy(ms, p, m->type, m->flag & INDIR, s, (uint32_t)(offset + o),
		return -1;

	if ((ms->flags & MAGIC_DEBUG) != 0) {
		fprintf(stderr, "mget(type=%d, flag=%x, offset=%u, o=%zu, "
		    "nbytes=%zu)\n", m->type, m->flag, offset, o, nbytes);
		mdebug(offset, (char *)(void *)p, sizeof(union VALUETYPE));
	}

	if (m->flag & INDIR) {
		case FILE_BESHORT:
			if (OFFSET_OOB(nbytes, offset, 2))
				return 0;
			if (off) {
				switch (m->in_op & FILE_OPS_MASK) {
				case FILE_OPAND:
					offset = (short)((p->hs[0]<<8)|
							 (p->hs[1])) &
						 off;
					break;
				case FILE_OPOR:
					offset = (short)((p->hs[0]<<8)|
							 (p->hs[1])) |
						 off;
					break;
				case FILE_OPXOR:
					offset = (short)((p->hs[0]<<8)|
							 (p->hs[1])) ^
						 off;
					break;
				case FILE_OPADD:
					offset = (short)((p->hs[0]<<8)|
							 (p->hs[1])) +
						 off;
					break;
				case FILE_OPMINUS:
					offset = (short)((p->hs[0]<<8)|
							 (p->hs[1])) -
						 off;
					break;
				case FILE_OPMULTIPLY:
					offset = (short)((p->hs[0]<<8)|
							 (p->hs[1])) *
						 off;
					break;
				case FILE_OPDIVIDE:
					offset = (short)((p->hs[0]<<8)|
							 (p->hs[1])) /
						 off;
					break;
				case FILE_OPMODULO:
					offset = (short)((p->hs[0]<<8)|
							 (p->hs[1])) %
						 off;
					break;
				}
			} else
				offset = (short)((p->hs[0]<<8)|
						 (p->hs[1]));
			if (m->in_op & FILE_OPINVERSE)
				offset = ~offset;
			break;
		case FILE_LESHORT:
			if (OFFSET_OOB(nbytes, offset, 2))
				return 0;
			if (off) {
				switch (m->in_op & FILE_OPS_MASK) {
				case FILE_OPAND:
					offset = (short)((p->hs[1]<<8)|
							 (p->hs[0])) &
						 off;
					break;
				case FILE_OPOR:
					offset = (short)((p->hs[1]<<8)|
							 (p->hs[0])) |
						 off;
					break;
				case FILE_OPXOR:
					offset = (short)((p->hs[1]<<8)|
							 (p->hs[0])) ^
						 off;
					break;
				case FILE_OPADD:
					offset = (short)((p->hs[1]<<8)|
							 (p->hs[0])) +
						 off;
					break;
				case FILE_OPMINUS:
					offset = (short)((p->hs[1]<<8)|
							 (p->hs[0])) -
						 off;
					break;
				case FILE_OPMULTIPLY:
					offset = (short)((p->hs[1]<<8)|
							 (p->hs[0])) *
						 off;
					break;
				case FILE_OPDIVIDE:
					offset = (short)((p->hs[1]<<8)|
							 (p->hs[0])) /
						 off;
					break;
				case FILE_OPMODULO:
					offset = (short)((p->hs[1]<<8)|
							 (p->hs[0])) %
						 off;
					break;
				}
			} else
				offset = (short)((p->hs[1]<<8)|
						 (p->hs[0]));
			if (m->in_op & FILE_OPINVERSE)
				offset = ~offset;
			break;
		case FILE_SHORT:
		case FILE_BEID3:
			if (OFFSET_OOB(nbytes, offset, 4))
				return 0;
			if (off) {
				switch (m->in_op & FILE_OPS_MASK) {
				case FILE_OPAND:
					offset = (int32_t)((p->hl[0]<<24)|
							 (p->hl[1]<<16)|
							 (p->hl[2]<<8)|
							 (p->hl[3])) &
						 off;
					break;
				case FILE_OPOR:
					offset = (int32_t)((p->hl[0]<<24)|
							 (p->hl[1]<<16)|
							 (p->hl[2]<<8)|
							 (p->hl[3])) |
						 off;
					break;
				case FILE_OPXOR:
					offset = (int32_t)((p->hl[0]<<24)|
							 (p->hl[1]<<16)|
							 (p->hl[2]<<8)|
							 (p->hl[3])) ^
						 off;
					break;
				case FILE_OPADD:
					offset = (int32_t)((p->hl[0]<<24)|
							 (p->hl[1]<<16)|
							 (p->hl[2]<<8)|
							 (p->hl[3])) +
						 off;
					break;
				case FILE_OPMINUS:
					offset = (int32_t)((p->hl[0]<<24)|
							 (p->hl[1]<<16)|
							 (p->hl[2]<<8)|
							 (p->hl[3])) -
						 off;
					break;
				case FILE_OPMULTIPLY:
					offset = (int32_t)((p->hl[0]<<24)|
							 (p->hl[1]<<16)|
							 (p->hl[2]<<8)|
							 (p->hl[3])) *
						 off;
					break;
				case FILE_OPDIVIDE:
					offset = (int32_t)((p->hl[0]<<24)|
							 (p->hl[1]<<16)|
							 (p->hl[2]<<8)|
							 (p->hl[3])) /
						 off;
					break;
				case FILE_OPMODULO:
					offset = (int32_t)((p->hl[0]<<24)|
							 (p->hl[1]<<16)|
							 (p->hl[2]<<8)|
							 (p->hl[3])) %
						 off;
					break;
				}
			} else
				offset = (int32_t)((p->hl[0]<<24)|
						 (p->hl[1]<<16)|
						 (p->hl[2]<<8)|
						 (p->hl[3]));
			if (m->in_op & FILE_OPINVERSE)
				offset = ~offset;
			break;
		case FILE_LELONG:
		case FILE_LEID3:
			if (OFFSET_OOB(nbytes, offset, 4))
				return 0;
			if (off) {
				switch (m->in_op & FILE_OPS_MASK) {
				case FILE_OPAND:
					offset = (int32_t)((p->hl[3]<<24)|
							 (p->hl[2]<<16)|
							 (p->hl[1]<<8)|
							 (p->hl[0])) &
						 off;
					break;
				case FILE_OPOR:
					offset = (int32_t)((p->hl[3]<<24)|
							 (p->hl[2]<<16)|
							 (p->hl[1]<<8)|
							 (p->hl[0])) |
						 off;
					break;
				case FILE_OPXOR:
					offset = (int32_t)((p->hl[3]<<24)|
							 (p->hl[2]<<16)|
							 (p->hl[1]<<8)|
							 (p->hl[0])) ^
						 off;
					break;
				case FILE_OPADD:
					offset = (int32_t)((p->hl[3]<<24)|
							 (p->hl[2]<<16)|
							 (p->hl[1]<<8)|
							 (p->hl[0])) +
						 off;
					break;
				case FILE_OPMINUS:
					offset = (int32_t)((p->hl[3]<<24)|
							 (p->hl[2]<<16)|
							 (p->hl[1]<<8)|
							 (p->hl[0])) -
						 off;
					break;
				case FILE_OPMULTIPLY:
					offset = (int32_t)((p->hl[3]<<24)|
							 (p->hl[2]<<16)|
							 (p->hl[1]<<8)|
							 (p->hl[0])) *
						 off;
					break;
				case FILE_OPDIVIDE:
					offset = (int32_t)((p->hl[3]<<24)|
							 (p->hl[2]<<16)|
							 (p->hl[1]<<8)|
							 (p->hl[0])) /
						 off;
					break;
				case FILE_OPMODULO:
					offset = (int32_t)((p->hl[3]<<24)|
							 (p->hl[2]<<16)|
							 (p->hl[1]<<8)|
							 (p->hl[0])) %
						 off;
					break;
				}
			} else
				offset = (int32_t)((p->hl[3]<<24)|
						 (p->hl[2]<<16)|
						 (p->hl[1]<<8)|
						 (p->hl[0]));
			if (m->in_op & FILE_OPINVERSE)
				offset = ~offset;
			break;
		case FILE_MELONG:
			if (OFFSET_OOB(nbytes, offset, 4))
				return 0;
			if (off) {
				switch (m->in_op & FILE_OPS_MASK) {
				case FILE_OPAND:
					offset = (int32_t)((p->hl[1]<<24)|
							 (p->hl[0]<<16)|
							 (p->hl[3]<<8)|
							 (p->hl[2])) &
						 off;
					break;
				case FILE_OPOR:
					offset = (int32_t)((p->hl[1]<<24)|
							 (p->hl[0]<<16)|
							 (p->hl[3]<<8)|
							 (p->hl[2])) |
						 off;
					break;
				case FILE_OPXOR:
					offset = (int32_t)((p->hl[1]<<24)|
							 (p->hl[0]<<16)|
							 (p->hl[3]<<8)|
							 (p->hl[2])) ^
						 off;
					break;
				case FILE_OPADD:
					offset = (int32_t)((p->hl[1]<<24)|
							 (p->hl[0]<<16)|
							 (p->hl[3]<<8)|
							 (p->hl[2])) +
						 off;
					break;
				case FILE_OPMINUS:
					offset = (int32_t)((p->hl[1]<<24)|
							 (p->hl[0]<<16)|
							 (p->hl[3]<<8)|
							 (p->hl[2])) -
						 off;
					break;
				case FILE_OPMULTIPLY:
					offset = (int32_t)((p->hl[1]<<24)|
							 (p->hl[0]<<16)|
							 (p->hl[3]<<8)|
							 (p->hl[2])) *
						 off;
					break;
				case FILE_OPDIVIDE:
					offset = (int32_t)((p->hl[1]<<24)|
							 (p->hl[0]<<16)|
							 (p->hl[3]<<8)|
							 (p->hl[2])) /
						 off;
					break;
				case FILE_OPMODULO:
					offset = (int32_t)((p->hl[1]<<24)|
							 (p->hl[0]<<16)|
							 (p->hl[3]<<8)|
							 (p->hl[2])) %
						 off;
					break;
				}
			} else
				offset = (int32_t)((p->hl[1]<<24)|
						 (p->hl[0]<<16)|
						 (p->hl[3]<<8)|
						 (p->hl[2]));
			if (m->in_op & FILE_OPINVERSE)
				offset = ~offset;
			break;
		case FILE_LONG:
			offset = ((((offset >>  0) & 0x7f) <<  0) |
				 (((offset >>  8) & 0x7f) <<  7) |
				 (((offset >> 16) & 0x7f) << 14) |
				 (((offset >> 24) & 0x7f) << 21)) + 10;
			break;
		default:
			break;
		}
		break;

	case FILE_INDIRECT:
		if (offset == 0)
			return 0;
		if (OFFSET_OOB(nbytes, offset, 0))
			return 0;
		sbuf = ms->o.buf;
		soffset = ms->offset;
		ms->o.buf = NULL;
		ms->offset = 0;
		rv = file_softmagic(ms, s + offset, nbytes - offset,
		    recursion_level, BINTEST, text);
		if ((ms->flags & MAGIC_DEBUG) != 0)
			fprintf(stderr, "indirect @offs=%u[%d]\n", offset, rv);
		rbuf = ms->o.buf;
		ms->o.buf = sbuf;
		ms->offset = soffset;
		if (rv == 1) {
			if ((ms->flags & (MAGIC_MIME|MAGIC_APPLE)) == 0 &&
			    file_printf(ms, m->desc, offset) == -1) {
				if (rbuf) {
					efree(rbuf);
				}
				return -1;
			}
			if (file_printf(ms, "%s", rbuf) == -1) {
				if (rbuf) {
					efree(rbuf);
				}
				return -1;
			}
		}
		if (rbuf) {
			efree(rbuf);
		}
		return rv;

	case FILE_USE:
		if (OFFSET_OOB(nbytes, offset, 0))
			return 0;
		sbuf = m->value.s;
		if (*sbuf == '^') {
			sbuf++;
			flip = !flip;
		}
		if (file_magicfind(ms, sbuf, &ml) == -1) {
			file_error(ms, 0, "cannot find entry `%s'", sbuf);
			return -1;
		}

		oneed_separator = *need_separator;
		if (m->flag & NOSPACE)
			*need_separator = 0;
		rv = match(ms, ml.magic, ml.nmagic, s, nbytes, offset + o,
		    mode, text, flip, recursion_level, printed_something,
		    need_separator, returnval);
		if (rv != 1)
		    *need_separator = oneed_separator;
		return rv;

	}
	t->val[j++] = '~';

	if (options & PCRE_CASELESS)
		t->val[j++] = 'i';

	if (options & PCRE_MULTILINE)
		t->val[j++] = 'm';
			break;

		default:
			matched = 0;
			file_magerror(ms, "cannot happen with float: invalid relation `%c'",
			    m->reln);
			return -1;
		}
			break;

		default:
			matched = 0;
			file_magerror(ms, "cannot happen with double: invalid relation `%c'", m->reln);
			return -1;
		}
		return matched;
			if (slen + idx > ms->search.s_len)
				break;

			v = file_strncmp(m->value.s, ms->search.s + idx, slen, m->str_flags);
			if (v == 0) {	/* found match */
				ms->search.offset += idx;
				break;
			}
		}
		break;
		break;

	default:
		matched = 0;
		file_magerror(ms, "cannot happen: invalid relation `%c'",
		    m->reln);
		return -1;
	}