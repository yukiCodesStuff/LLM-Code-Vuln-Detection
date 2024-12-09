#include "file.h"

#ifndef lint
FILE_RCSID("@(#)$File: cdf.c,v 1.55 2014/02/27 23:26:17 christos Exp $")
#endif

#include <assert.h>
#ifdef CDF_DEBUG
#define CDF_TOLE8(x)	((uint64_t)(NEED_SWAP ? _cdf_tole8(x) : (uint64_t)(x)))
#define CDF_TOLE4(x)	((uint32_t)(NEED_SWAP ? _cdf_tole4(x) : (uint32_t)(x)))
#define CDF_TOLE2(x)	((uint16_t)(NEED_SWAP ? _cdf_tole2(x) : (uint16_t)(x)))
#define CDF_GETUINT32(x, y)	cdf_getuint32(x, y)


/*
	const char *e = ((const char *)p) + tail;
	size_t ss = sst->sst_dirlen < h->h_min_size_standard_stream ?
	    CDF_SHORT_SEC_SIZE(h) : CDF_SEC_SIZE(h);
	(void)&line;
	if (e >= b && (size_t)(e - b) <= ss * sst->sst_len)
		return 0;
	DPRINTF(("%d: offset begin %p < end %p || %" SIZE_T_FORMAT "u"
	    " > %" SIZE_T_FORMAT "u [%" SIZE_T_FORMAT "u %"
	    / sizeof(maxsector));

	DPRINTF(("Chain:"));
	for (j = i = 0; sid >= 0; i++, j++) {
		DPRINTF((" %d", sid));
		if (j >= CDF_LOOP_LIMIT) {
			DPRINTF(("Counting chain loop limit"));
		}
		sid = CDF_TOLE4((uint32_t)sat->sat_tab[sid]);
	}
	DPRINTF(("\n"));
	return i;
}

    const cdf_sat_t *sat, const cdf_sat_t *ssat, const cdf_stream_t *sst,
    const cdf_dir_t *dir, cdf_stream_t *scn)
{
	size_t i;
	const cdf_directory_t *d;
	static const char name[] = "\05SummaryInformation";

	for (i = dir->dir_len; i > 0; i--)
		if (dir->dir_tab[i - 1].d_type == CDF_DIR_TYPE_USER_STREAM &&
		    cdf_namecmp(name, dir->dir_tab[i - 1].d_name, sizeof(name))
		    == 0)
			break;

	if (i == 0) {
		DPRINTF(("Cannot find summary information section\n"));
		errno = ESRCH;
		return -1;
	}
	d = &dir->dir_tab[i - 1];
	return cdf_read_sector_chain(info, h, sat, ssat, sst,
	    d->d_stream_first_sector, d->d_size, scn);
}

int
cdf_read_property_info(const cdf_stream_t *sst, const cdf_header_t *h,
    uint32_t offs, cdf_property_info_t **info, size_t *count, size_t *maxcount)
{
	if (cdf_check_stream_offset(sst, h, e, 0, __LINE__) == -1)
		goto out;
	for (i = 0; i < sh.sh_properties; i++) {
		size_t ofs, tail = (i << 1) + 1;
		if (cdf_check_stream_offset(sst, h, p, tail * sizeof(uint32_t),
		    __LINE__) == -1)
			goto out;
		ofs = CDF_GETUINT32(p, tail);
		q = (const uint8_t *)(const void *)
		    ((const char *)(const void *)p + ofs
		    - 2 * sizeof(uint32_t));
		if (q < p || q > e) {
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



int
cdf_print_classid(char *buf, size_t buflen, const cdf_classid_t *id)
{
	return len;
}


#ifdef CDF_DEBUG
void
cdf_dump_header(const cdf_header_t *h)
	for (i = 0; i < __arraycount(h->h_master_sat); i++) {
		if (h->h_master_sat[i] == CDF_SECID_FREE)
			break;
		(void)fprintf(stderr, "%35.35s[%.3zu] = %d\n",
		    "master_sat", i, h->h_master_sat[i]);
	}
}

}

void
cdf_dump(void *v, size_t len)
{
	size_t i, j;
	unsigned char *p = v;
	char abuf[16];
	(void)fprintf(stderr, "%.4x: ", 0);
	for (i = 0, j = 0; i < len; i++, p++) {
		(void)fprintf(stderr, "%.2x ", *p);
		abuf[j++] = isprint(*p) ? *p : '.';
	    "user stream", "lockbytes", "property", "root storage" };

	for (i = 0; i < dir->dir_len; i++) {
		d = &dir->dir_tab[i];
		for (j = 0; j < sizeof(name); j++)
			name[j] = (char)CDF_TOLE2(d->d_name[j]);
		(void)fprintf(stderr, "Directory %" SIZE_T_FORMAT "u: %s\n",
		(void)fprintf(stderr, "Right child: %d\n", d->d_right_child);
		(void)fprintf(stderr, "Flags: 0x%x\n", d->d_flags);
		cdf_timestamp_to_timespec(&ts, d->d_created);
		(void)fprintf(stderr, "Created %s", cdf_ctime(&ts.tv_sec));
		cdf_timestamp_to_timespec(&ts, d->d_modified);
		(void)fprintf(stderr, "Modified %s", cdf_ctime(&ts.tv_sec));
		(void)fprintf(stderr, "Stream %d\n", d->d_stream_first_sector);
		(void)fprintf(stderr, "Size %d\n", d->d_size);
		switch (d->d_type) {
		case CDF_DIR_TYPE_USER_STORAGE:
				cdf_print_elapsed_time(buf, sizeof(buf), tp);
				(void)fprintf(stderr, "timestamp %s\n", buf);
			} else {
				cdf_timestamp_to_timespec(&ts, tp);
				(void)fprintf(stderr, "timestamp %s",
				    cdf_ctime(&ts.tv_sec));
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

#endif

#ifdef TEST
int
	cdf_stream_t sst, scn;
	cdf_dir_t dir;
	cdf_info_t info;

	if (argc < 2) {
		(void)fprintf(stderr, "Usage: %s <filename>\n", getprogname());
		return -1;
		if (cdf_read_dir(&info, &h, &sat, &dir) == -1)
			err(1, "Cannot read dir");

		if (cdf_read_short_stream(&info, &h, &sat, &dir, &sst) == -1)
			err(1, "Cannot read short stream");
#ifdef CDF_DEBUG
		cdf_dump_stream(&h, &sst);
#endif

		if (cdf_read_summary_info(&info, &h, &sat, &ssat, &sst, &dir,
		    &scn) == -1)
			err(1, "Cannot read summary info");
#ifdef CDF_DEBUG
		cdf_dump_summary_info(&h, &scn);
#endif

		(void)close(info.i_fd);
	}