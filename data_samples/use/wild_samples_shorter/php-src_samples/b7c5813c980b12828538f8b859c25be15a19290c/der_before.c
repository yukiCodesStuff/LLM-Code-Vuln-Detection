#include "file.h"

#ifndef lint
FILE_RCSID("@(#)$File: der.c,v 1.24 2022/07/30 18:08:36 christos Exp $")
#endif
#else
#define SIZE_T_FORMAT "z"
#define CAST(a, b) ((a)(b))
		DPRINTF(("%s: bad tag 1\n", __func__));
		return -1;
	}
	DPRINTF(("%s1: %d %" SIZE_T_FORMAT "u %u\n", __func__, ms->offset,
	    offs, m->offset));

	uint32_t tlen = getlength(b, &offs, len);
	if (tlen == DER_BAD) {
		DPRINTF(("%s: bad tag 2\n", __func__));
		return -1;
	}
	DPRINTF(("%s2: %d %" SIZE_T_FORMAT "u %u\n", __func__, ms->offset,
	    offs, tlen));

	offs += ms->offset + m->offset;
	DPRINTF(("cont_level = %d\n", m->cont_level));
#ifdef DEBUG_DER
	size_t i;
	for (i = 0; i < m->cont_level; i++)
		printf("cont_level[%" SIZE_T_FORMAT "u] = %u\n", i,
		    ms->c.li[i].off);
#endif
	if (m->cont_level != 0) {
		if (offs + tlen > nbytes)
			return -1;
		ms->c.li[m->cont_level - 1].off = CAST(int, offs + tlen);
		DPRINTF(("cont_level[%u] = %u\n", m->cont_level - 1,
		    ms->c.li[m->cont_level - 1].off));
	}
	return CAST(int32_t, offs);
}
		return -1;
	}

	DPRINTF(("%s1: %d %" SIZE_T_FORMAT "u %u\n", __func__, ms->offset,
	    offs, m->offset));

	tlen = getlength(b, &offs, len);
	if (tlen == DER_BAD) {