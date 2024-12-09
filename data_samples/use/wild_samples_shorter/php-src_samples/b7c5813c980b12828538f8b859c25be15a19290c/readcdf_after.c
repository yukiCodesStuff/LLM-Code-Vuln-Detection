#include "file.h"

#ifndef lint
FILE_RCSID("@(#)$File: readcdf.c,v 1.80 2023/01/24 20:13:40 christos Exp $")
#endif

#include <assert.h>
#include <stdlib.h>
	},
};

file_private const char *
cdf_clsid_to_mime(const uint64_t clsid[2], const struct cv *cv)
{
	size_t i;
	for (i = 0; cv[i].mime != NULL; i++) {
	return NULL;
}

file_private const char *
cdf_app_to_mime(const char *vbuf, const struct nv *nv)
{
	size_t i;
	const char *rv = NULL;
	return rv;
}

file_private int
cdf_file_property_info(struct magic_set *ms, const cdf_property_info_t *info,
    size_t count, const cdf_directory_t *root_storage)
{
	size_t i;
	return 1;
}

file_private int
cdf_file_catalog(struct magic_set *ms, const cdf_header_t *h,
    const cdf_stream_t *sst)
{
	cdf_catalog_t *cat;
	return 1;
}

file_private int
cdf_file_summary_info(struct magic_set *ms, const cdf_header_t *h,
    const cdf_stream_t *sst, const cdf_directory_t *root_storage)
{
	cdf_summary_info_header_t si;
}

#ifdef notdef
file_private char *
format_clsid(char *buf, size_t len, const uint64_t uuid[2]) {
	snprintf(buf, len, "%.8" PRIx64 "-%.4" PRIx64 "-%.4" PRIx64 "-%.4"
	    PRIx64 "-%.12" PRIx64,
	    (uuid[0] >> 32) & (uint64_t)0x000000000ffffffffULL,
}
#endif

file_private int
cdf_file_catalog_info(struct magic_set *ms, const cdf_info_t *info,
    const cdf_header_t *h, const cdf_sat_t *sat, const cdf_sat_t *ssat,
    const cdf_stream_t *sst, const cdf_dir_t *dir, cdf_stream_t *scn)
{
	return i;
}

file_private int
cdf_check_summary_info(struct magic_set *ms, const cdf_info_t *info,
    const cdf_header_t *h, const cdf_sat_t *sat, const cdf_sat_t *ssat,
    const cdf_stream_t *sst, const cdf_dir_t *dir, cdf_stream_t *scn,
    const cdf_directory_t *root_storage, const char **expn)
	return i;
}

file_private struct sinfo {
	const char *name;
	const char *mime;
	const char *sections[5];
	const int  types[5];
	},
};

file_private int
cdf_file_dir_info(struct magic_set *ms, const cdf_dir_t *dir)
{
	size_t sd, j;

	return -1;
}

file_protected int
file_trycdf(struct magic_set *ms, const struct buffer *b)
{
	int fd = b->fd;
	const unsigned char *buf = CAST(const unsigned char *, b->fbuf);
		    sizeof(HWP5_SIGNATURE) - 1) == 0) {
		    if (NOTMIME(ms)) {
			if (file_printf(ms,
			    "Hancom HWP (Hangul Word Processor) file, version 5.0") == -1)
			    return -1;
		    } else if (ms->flags & MAGIC_MIME_TYPE) {
			if (file_printf(ms, "application/x-hwp") == -1)
			    return -1;