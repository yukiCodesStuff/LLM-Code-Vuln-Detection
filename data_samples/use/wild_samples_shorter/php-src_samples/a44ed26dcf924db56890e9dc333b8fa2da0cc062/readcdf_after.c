#include "file.h"

#ifndef lint
FILE_RCSID("@(#)$File: readcdf.c,v 1.50 2015/01/02 21:29:39 christos Exp $")
#endif

#include <stdlib.h>
#ifdef PHP_WIN32
#include "cdf.h"
#include "magic.h"

#ifndef __arraycount
#define __arraycount(a) (sizeof(a) / sizeof(a[0]))
#endif

#define NOTMIME(ms) (((ms)->flags & MAGIC_MIME) == 0)

static const struct nv {
	const char *pattern;
	},
	{	{ 0,			 0			},
		NULL,
	},
};

private const char *
cdf_clsid_to_mime(const uint64_t clsid[2], const struct cv *cv)
	memset(&ts, 0, sizeof(ts));

        if (!NOTMIME(ms) && root_storage)
		str = cdf_clsid_to_mime(root_storage->d_storage_uuid,
		    clsid2mime);

        for (i = 0; i < count; i++) {
                cdf_print_property_name(buf, sizeof(buf), info[i].pi_id);
                switch (info[i].pi_type) {
                                if (info[i].pi_type == CDF_LENGTH32_WSTRING)
                                    k++;
                                s = info[i].pi_str.s_buf;
                                for (j = 0; j < sizeof(vbuf) && len--; s += k) {
                                        if (*s == '\0')
                                                break;
                                        if (isprint((unsigned char)*s))
                                                vbuf[j++] = *s;
                                }
                                if (j == sizeof(vbuf))
                                        --j;
                                vbuf[j] = '\0';
        return 1;
}

private int
cdf_file_catalog(struct magic_set *ms, const cdf_header_t *h,
    const cdf_stream_t *sst)
{
	cdf_catalog_t *cat;
	size_t i;
	char buf[256];
	cdf_catalog_entry_t *ce;

        if (NOTMIME(ms)) {
		if (file_printf(ms, "Microsoft Thumbs.db [") == -1)
			return -1;
		if (cdf_unpack_catalog(h, sst, &cat) == -1)
			return -1;
		ce = cat->cat_e;
		/* skip first entry since it has a , or paren */
		for (i = 1; i < cat->cat_num; i++)
			if (file_printf(ms, "%s%s",
			    cdf_u16tos8(buf, ce[i].ce_namlen, ce[i].ce_name),
			    i == cat->cat_num - 1 ? "]" : ", ") == -1) {
				free(cat);
				return -1;
			}
		free(cat);
	} else {
		if (file_printf(ms, "application/CDFV2") == -1)
			return -1;
	}
	return 1;
}

private int
cdf_file_summary_info(struct magic_set *ms, const cdf_header_t *h,
    const cdf_stream_t *sst, const cdf_directory_t *root_storage)
{
                        break;
                }
		if (root_storage) {
			str = cdf_clsid_to_mime(root_storage->d_storage_uuid,
			    clsid2desc);
			if (str) {
				if (file_printf(ms, ", %s", str) == -1)
					return -2;
			}
		}
	}

        m = cdf_file_property_info(ms, info, count, root_storage);
        free(info);

        return m == -1 ? -2 : m;
}

#ifdef notdef
private char *
format_clsid(char *buf, size_t len, const uint64_t uuid[2]) {
	snprintf(buf, len, "%.8" PRIx64 "-%.4" PRIx64 "-%.4" PRIx64 "-%.4" 
	    PRIx64 "-%.12" PRIx64,
	    (uuid[0] >> 32) & (uint64_t)0x000000000ffffffffULL,
	    (uuid[0] >> 16) & (uint64_t)0x0000000000000ffffULL,
	    (uuid[0] >>  0) & (uint64_t)0x0000000000000ffffULL, 
	    (uuid[1] >> 48) & (uint64_t)0x0000000000000ffffULL,
	    (uuid[1] >>  0) & (uint64_t)0x0000fffffffffffffULL);
	return buf;
}
#endif

private int
cdf_file_catalog_info(struct magic_set *ms, const cdf_info_t *info,
    const cdf_header_t *h, const cdf_sat_t *sat, const cdf_sat_t *ssat,
    const cdf_stream_t *sst, const cdf_dir_t *dir, cdf_stream_t *scn)
{
	int i;

	if ((i = cdf_read_user_stream(info, h, sat, ssat, sst,
	    dir, "Catalog", scn)) <= 0)
		return i;
#ifdef CDF_DEBUG
	cdf_dump_catalog(&h, &scn);
#endif
	if ((i = cdf_file_catalog(ms, h, scn)) == -1)
		return -1;
	return i;
}

private struct sinfo {
	const char *name;
	const char *mime;
	const char *sections[5];
	const int  types[5];
} sectioninfo[] = {
	{ "Encrypted", "encrypted", 
		{
			"EncryptedPackage", NULL, NULL, NULL, NULL,
		},
		{
			CDF_DIR_TYPE_USER_STREAM, 0, 0, 0, 0,

		},
	},
	{ "QuickBooks", "quickbooks", 
		{
#if 0
			"TaxForms", "PDFTaxForms", "modulesInBackup",
#endif
			"mfbu_header", NULL, NULL, NULL, NULL,
		},
		{
#if 0
			CDF_DIR_TYPE_USER_STORAGE,
			CDF_DIR_TYPE_USER_STORAGE,
			CDF_DIR_TYPE_USER_STREAM,
#endif
			CDF_DIR_TYPE_USER_STREAM,
			0, 0, 0, 0
		},
	},
};

private int
cdf_file_dir_info(struct magic_set *ms, const cdf_dir_t *dir)
{
	size_t sd, j;

	for (sd = 0; sd < __arraycount(sectioninfo); sd++) {
		const struct sinfo *si = &sectioninfo[sd];
		for (j = 0; si->sections[j]; j++) {
			if (cdf_find_stream(dir, si->sections[j], si->types[j])
			    <= 0) {
#ifdef CDF_DEBUG
				fprintf(stderr, "Can't read %s\n",
				    si->sections[j]);
#endif
				break;
			}
		}
		if (si->sections[j] != NULL)
			continue;
		if (NOTMIME(ms)) {
			if (file_printf(ms, "CDFV2 %s", si->name) == -1)
				return -1;
		} else {
			if (file_printf(ms, "application/CDFV2-%s",
			    si->mime) == -1)
				return -1;
		}
		return 1;
	}
	return -1;
}

protected int
file_trycdf(struct magic_set *ms, int fd, const unsigned char *buf,
    size_t nbytes)
{
        cdf_dir_t dir;
        int i;
        const char *expn = "";
        const cdf_directory_t *root_storage;

        info.i_fd = fd;
        info.i_buf = buf;
#ifdef CDF_DEBUG
        cdf_dump_dir(&info, &h, &sat, &ssat, &sst, &dir);
#endif
#ifdef notdef
	if (root_storage) {
		if (NOTMIME(ms)) {
			char clsbuf[128];
			if (file_printf(ms, "CLSID %s, ",
			    format_clsid(clsbuf, sizeof(clsbuf),
			    root_storage->d_storage_uuid)) == -1)
				return -1;
		}
	}
#endif

	if ((i = cdf_read_user_stream(&info, &h, &sat, &ssat, &sst, &dir,
	    "FileHeader", &scn)) != -1) {
#define HWP5_SIGNATURE "HWP Document File"
		if (scn.sst_dirlen >= sizeof(HWP5_SIGNATURE) - 1
		    && memcmp(scn.sst_tab, HWP5_SIGNATURE,
		    sizeof(HWP5_SIGNATURE) - 1) == 0) {
		    if (NOTMIME(ms)) {
			if (file_printf(ms,
			    "Hangul (Korean) Word Processor File 5.x") == -1)
			    return -1;
		    } else {
			if (file_printf(ms, "application/x-hwp") == -1)
			    return -1;
		    }
		    i = 1;
		    goto out5;
		} else {
		    free(scn.sst_tab);
		    scn.sst_tab = NULL;
		    scn.sst_len = 0;
		    scn.sst_dirlen = 0;
		}
	}

        if ((i = cdf_read_summary_info(&info, &h, &sat, &ssat, &sst, &dir,
            &scn)) == -1) {
                if (errno != ESRCH) {
                        expn = "Cannot read summary info";
			goto out4;
		}
		i = cdf_file_catalog_info(ms, &info, &h, &sat, &ssat, &sst,
		    &dir, &scn);
		if (i > 0)
			goto out4;
		i = cdf_file_dir_info(ms, &dir);
		if (i < 0)
                        expn = "Cannot read section info";
		goto out4;
	}


#ifdef CDF_DEBUG
        cdf_dump_summary_info(&h, &scn);
#endif
        if ((i = cdf_file_summary_info(ms, &h, &scn, root_storage)) < 0)
			i = 1;
		}
	}
out5:
        free(scn.sst_tab);
out4:
        free(sst.sst_tab);
out3:
		    "Composite Document File V2 Document") == -1)
		    return -1;
		if (*expn)
		    if (file_printf(ms, ", %s", expn) == -1)
			return -1;
	    } else {
		if (file_printf(ms, "application/CDFV2-unknown") == -1)
		    return -1;
	    }
	    i = 1;
	}