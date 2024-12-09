			const char *last;	/* end of search region */
			const char *buf;	/* start of search region */
			const char *end;
			size_t lines, linecnt, bytecnt, bytecnt_max;

			if (s == NULL) {
				ms->search.s_len = 0;
				ms->search.s = NULL;
				bytecnt = m->str_range;
			}

			/* XXX bytecnt_max is to be kept for PHP, see cve-2014-3538.
				PCRE might stuck if the input buffer is too big. To ensure
				the correctness, the check for bytecnt > nbytes is also
				kept (might be abundant). */
			bytecnt_max = nbytes - offset;
			bytecnt_max = bytecnt_max > (1 << 14) ? (1 << 14) : bytecnt_max;
			bytecnt_max = bytecnt > nbytes ? nbytes : bytecnt_max;
			if (bytecnt == 0 || bytecnt > bytecnt_max)
				bytecnt = bytecnt_max;

			buf = RCAST(const char *, s) + offset;
			end = last = RCAST(const char *, s) + bytecnt;
			/* mget() guarantees buf <= last */