			const char *last;	/* end of search region */
			const char *buf;	/* start of search region */
			const char *end;
			size_t lines, linecnt, bytecnt;

			if (s == NULL) {
				ms->search.s_len = 0;
				ms->search.s = NULL;
				bytecnt = m->str_range;
			}

			if (bytecnt == 0 || bytecnt > nbytes - offset)
				bytecnt = nbytes - offset;

			buf = RCAST(const char *, s) + offset;
			end = last = RCAST(const char *, s) + bytecnt;
			/* mget() guarantees buf <= last */