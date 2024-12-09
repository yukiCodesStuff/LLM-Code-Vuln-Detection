#include "file.h"

#ifndef lint
FILE_RCSID("@(#)$File: cdf.c,v 1.73 2015/01/11 16:58:25 christos Exp $")
#endif

#include <assert.h>
#ifdef CDF_DEBUG
#define CDF_TOLE8(x)	((uint64_t)(NEED_SWAP ? _cdf_tole8(x) : (uint64_t)(x)))
#define CDF_TOLE4(x)	((uint32_t)(NEED_SWAP ? _cdf_tole4(x) : (uint32_t)(x)))
#define CDF_TOLE2(x)	((uint16_t)(NEED_SWAP ? _cdf_tole2(x) : (uint16_t)(x)))
#define CDF_TOLE(x)	(/*CONSTCOND*/sizeof(x) == 2 ? \
			    CDF_TOLE2(CAST(uint16_t, x)) : \
			(/*CONSTCOND*/sizeof(x) == 4 ? \
			    CDF_TOLE4(CAST(uint32_t, x)) : \
			    CDF_TOLE8(CAST(uint64_t, x))))
#define CDF_GETUINT32(x, y)	cdf_getuint32(x, y)


/*
	const char *e = ((const char *)p) + tail;
	size_t ss = sst->sst_dirlen < h->h_min_size_standard_stream ?
	    CDF_SHORT_SEC_SIZE(h) : CDF_SEC_SIZE(h);
	/*LINTED*/(void)&line;
	if (e >= b && (size_t)(e - b) <= ss * sst->sst_len)
		return 0;
	DPRINTF(("%d: offset begin %p < end %p || %" SIZE_T_FORMAT "u"
	    " > %" SIZE_T_FORMAT "u [%" SIZE_T_FORMAT "u %"
	    / sizeof(maxsector));

	DPRINTF(("Chain:"));
	if (sid == CDF_SECID_END_OF_CHAIN) {
		/* 0-length chain. */
		DPRINTF((" empty\n"));
		return 0;
	}

	for (j = i = 0; sid >= 0; i++, j++) {
		DPRINTF((" %d", sid));
		if (j >= CDF_LOOP_LIMIT) {
			DPRINTF(("Counting chain loop limit"));
		}
		sid = CDF_TOLE4((uint32_t)sat->sat_tab[sid]);
	}
	if (i == 0) {
		DPRINTF((" none, sid: %d\n", sid));
		return (size_t)-1;

	}
	DPRINTF(("\n"));
	return i;
}

    const cdf_sat_t *sat, const cdf_sat_t *ssat, const cdf_stream_t *sst,
    const cdf_dir_t *dir, cdf_stream_t *scn)
{
	return cdf_read_user_stream(info, h, sat, ssat, sst, dir,
	    "\05SummaryInformation", scn);
}

int
cdf_read_user_stream(const cdf_info_t *info, const cdf_header_t *h,
    const cdf_sat_t *sat, const cdf_sat_t *ssat, const cdf_stream_t *sst,
    const cdf_dir_t *dir, const char *name, cdf_stream_t *scn)
{
	const cdf_directory_t *d;
	int i = cdf_find_stream(dir, name, CDF_DIR_TYPE_USER_STREAM);

	if (i <= 0)
		return -1;

	d = &dir->dir_tab[i - 1];
	return cdf_read_sector_chain(info, h, sat, ssat, sst,
	    d->d_stream_first_sector, d->d_size, scn);
}

int
cdf_find_stream(const cdf_dir_t *dir, const char *name, int type)
{
	size_t i, name_len = strlen(name) + 1;

	for (i = dir->dir_len; i > 0; i--)
		if (dir->dir_tab[i - 1].d_type == type &&
		    cdf_namecmp(name, dir->dir_tab[i - 1].d_name, name_len)
		    == 0)
			break;
	if (i > 0)
		return i;

	DPRINTF(("Cannot find type %d `%s'\n", type, name));
	errno = ESRCH;
	return 0;
}

int
cdf_read_property_info(const cdf_stream_t *sst, const cdf_header_t *h,
    uint32_t offs, cdf_property_info_t **info, size_t *count, size_t *maxcount)
{
	if (cdf_check_stream_offset(sst, h, e, 0, __LINE__) == -1)
		goto out;
	for (i = 0; i < sh.sh_properties; i++) {
		size_t tail = (i << 1) + 1;
		size_t ofs;
		if (cdf_check_stream_offset(sst, h, p, tail * sizeof(uint32_t),
		    __LINE__) == -1)
			goto out;
		ofs = CDF_GETUINT32(p, tail);
		q = (const uint8_t *)(const void *)
		    ((const char *)(const void *)p + ofs
		    - 2 * sizeof(uint32_t));
		if (q < p) {
			DPRINTF(("Wrapped around %p < %p\n", q, p));
			goto out;
		}
		if (q > e) {
			DPRINTF(("Ran of the end %p > %p\n", q, e));
			goto out;
		}
		inp[i].pi_id = CDF_GETUINT32(p, i << 1);
	maxcount = 0;
	*info = NULL;
	if (cdf_read_property_info(sst, h, CDF_TOLE4(sd->sd_offset), info,
	    count, &maxcount) == -1)
		return -1;
	return 0;
}


#define extract_catalog_field(t, f, l) \
    if (b + l + sizeof(cep->f) > eb) { \
	    cep->ce_namlen = 0; \
	    break; \
    } \
    memcpy(&cep->f, b + (l), sizeof(cep->f)); \
    ce[i].f = CAST(t, CDF_TOLE(cep->f))

int
cdf_unpack_catalog(const cdf_header_t *h, const cdf_stream_t *sst,
    cdf_catalog_t **cat)
{
	size_t ss = sst->sst_dirlen < h->h_min_size_standard_stream ?
	    CDF_SHORT_SEC_SIZE(h) : CDF_SEC_SIZE(h);
	const char *b = CAST(const char *, sst->sst_tab);
	const char *eb = b + ss * sst->sst_len;
	size_t nr, i, k;
	cdf_catalog_entry_t *ce;
	uint16_t reclen;
	const uint16_t *np;

	for (nr = 0; b < eb; nr++) {
		memcpy(&reclen, b, sizeof(reclen));
		reclen = CDF_TOLE2(reclen);
		if (reclen == 0)
			break;
		b += reclen;
	}
	*cat = CAST(cdf_catalog_t *,
	    malloc(sizeof(cdf_catalog_t) + nr * sizeof(*ce)));
	(*cat)->cat_num = nr;
	ce = (*cat)->cat_e;
	memset(ce, 0, nr * sizeof(*ce));
	b = CAST(const char *, sst->sst_tab);
	for (i = 0; i < nr; i++, b += reclen) {
		cdf_catalog_entry_t *cep = &ce[i];
		uint16_t rlen;

		extract_catalog_field(uint16_t, ce_namlen, 0);
		extract_catalog_field(uint16_t, ce_num, 2);
		extract_catalog_field(uint64_t, ce_timestamp, 6);
		reclen = cep->ce_namlen;

		if (reclen < 14) {
			cep->ce_namlen = 0;
			continue;
		}

		cep->ce_namlen = __arraycount(cep->ce_name) - 1;
		rlen = reclen - 14;
		if (cep->ce_namlen > rlen)
			cep->ce_namlen = rlen;

		np = CAST(const uint16_t *, CAST(const void *, (b + 16)));
		if (CAST(const char *, np + cep->ce_namlen) > eb) {
			cep->ce_namlen = 0;
			break;
		}

		for (k = 0; k < cep->ce_namlen; k++)
			cep->ce_name[k] = np[k]; /* XXX: CDF_TOLE2? */
		cep->ce_name[cep->ce_namlen] = 0;
	}
	return 0;
}

int
cdf_print_classid(char *buf, size_t buflen, const cdf_classid_t *id)
{
	return len;
}

char *
cdf_u16tos8(char *buf, size_t len, const uint16_t *p)
{
	size_t i;
	for (i = 0; i < len && p[i]; i++)
		buf[i] = (char)p[i];
	buf[i] = '\0';
	return buf;
}

#ifdef CDF_DEBUG
void
cdf_dump_header(const cdf_header_t *h)
	for (i = 0; i < __arraycount(h->h_master_sat); i++) {
		if (h->h_master_sat[i] == CDF_SECID_FREE)
			break;
		(void)fprintf(stderr, "%35.35s[%.3" SIZE_T_FORMAT "u] = %d\n",
		    "master_sat", i, h->h_master_sat[i]);
	}
}

}

void
cdf_dump(const void *v, size_t len)
{
	size_t i, j;
	const unsigned char *p = v;
	char abuf[16];

	(void)fprintf(stderr, "%.4x: ", 0);
	for (i = 0, j = 0; i < len; i++, p++) {
		(void)fprintf(stderr, "%.2x ", *p);
		abuf[j++] = isprint(*p) ? *p : '.';
	    "user stream", "lockbytes", "property", "root storage" };

	for (i = 0; i < dir->dir_len; i++) {
		char buf[26];
		d = &dir->dir_tab[i];
		for (j = 0; j < sizeof(name); j++)
			name[j] = (char)CDF_TOLE2(d->d_name[j]);
		(void)fprintf(stderr, "Directory %" SIZE_T_FORMAT "u: %s\n",
		(void)fprintf(stderr, "Right child: %d\n", d->d_right_child);
		(void)fprintf(stderr, "Flags: 0x%x\n", d->d_flags);
		cdf_timestamp_to_timespec(&ts, d->d_created);
		(void)fprintf(stderr, "Created %s", cdf_ctime(&ts.tv_sec, buf));
		cdf_timestamp_to_timespec(&ts, d->d_modified);
		(void)fprintf(stderr, "Modified %s",
		    cdf_ctime(&ts.tv_sec, buf));
		(void)fprintf(stderr, "Stream %d\n", d->d_stream_first_sector);
		(void)fprintf(stderr, "Size %d\n", d->d_size);
		switch (d->d_type) {
		case CDF_DIR_TYPE_USER_STORAGE:
				cdf_print_elapsed_time(buf, sizeof(buf), tp);
				(void)fprintf(stderr, "timestamp %s\n", buf);
			} else {
				char tbuf[26];
				cdf_timestamp_to_timespec(&ts, tp);
				(void)fprintf(stderr, "timestamp %s",
				    cdf_ctime(&ts.tv_sec, tbuf));
			}
			break;
		case CDF_CLIPBOARD:
			(void)fprintf(stderr, "CLIPBOARD %u\n", info[i].pi_u32);
		return;
	(void)fprintf(stderr, "Endian: %x\n", ssi.si_byte_order);
	(void)fprintf(stderr, "Os Version %d.%d\n", ssi.si_os_version & 0xff,
	    ssi.si_os_version >> 8);
	(void)fprintf(stderr, "Os %d\n", ssi.si_os);
	cdf_print_classid(buf, sizeof(buf), &ssi.si_class);
	(void)fprintf(stderr, "Class %s\n", buf);
	(void)fprintf(stderr, "Count %d\n", ssi.si_count);
	free(info);
}


void
cdf_dump_catalog(const cdf_header_t *h, const cdf_stream_t *sst)
{
	cdf_catalog_t *cat;
	cdf_unpack_catalog(h, sst, &cat);
	const cdf_catalog_entry_t *ce = cat->cat_e;
	struct timespec ts;
	char tbuf[64], sbuf[256];
	size_t i;

	printf("Catalog:\n");
	for (i = 0; i < cat->cat_num; i++) {
		cdf_timestamp_to_timespec(&ts, ce[i].ce_timestamp);
		printf("\t%d %s %s", ce[i].ce_num,
		    cdf_u16tos8(sbuf, ce[i].ce_namlen, ce[i].ce_name),
		    cdf_ctime(&ts.tv_sec, tbuf));
	}
	free(cat);
}

#endif

#ifdef TEST
int
	cdf_stream_t sst, scn;
	cdf_dir_t dir;
	cdf_info_t info;
	const cdf_directory_t *root;

	if (argc < 2) {
		(void)fprintf(stderr, "Usage: %s <filename>\n", getprogname());
		return -1;
		if (cdf_read_dir(&info, &h, &sat, &dir) == -1)
			err(1, "Cannot read dir");

		if (cdf_read_short_stream(&info, &h, &sat, &dir, &sst, &root)
		    == -1)
			err(1, "Cannot read short stream");
#ifdef CDF_DEBUG
		cdf_dump_stream(&h, &sst);
#endif

		if (cdf_read_summary_info(&info, &h, &sat, &ssat, &sst, &dir,
		    &scn) == -1)
			warn("Cannot read summary info");
#ifdef CDF_DEBUG
		else
			cdf_dump_summary_info(&h, &scn);
#endif
		if (cdf_read_catalog(&info, &h, &sat, &ssat, &sst, &dir,
		    &scn) == -1)
			warn("Cannot read catalog");
#ifdef CDF_DEBUG
		else
			cdf_dump_catalog(&h, &scn);
#endif

		(void)close(info.i_fd);
	}