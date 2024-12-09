#include "file.h"

#ifndef lint
FILE_RCSID("@(#)$File: readcdf.c,v 1.40 2014/03/06 15:23:33 christos Exp $")
#endif

#include <stdlib.h>
#ifdef PHP_WIN32
#include "cdf.h"
#include "magic.h"

#define NOTMIME(ms) (((ms)->flags & MAGIC_MIME) == 0)

static const struct nv {
	const char *pattern;
	},
	{	{ 0,			 0			},
		NULL,
	}
};

private const char *
cdf_clsid_to_mime(const uint64_t clsid[2], const struct cv *cv)
	memset(&ts, 0, sizeof(ts));

        if (!NOTMIME(ms) && root_storage)
		str = cdf_clsid_to_mime(root_storage->d_storage_uuid, clsid2mime);

        for (i = 0; i < count; i++) {
                cdf_print_property_name(buf, sizeof(buf), info[i].pi_id);
                switch (info[i].pi_type) {
                                if (info[i].pi_type == CDF_LENGTH32_WSTRING)
                                    k++;
                                s = info[i].pi_str.s_buf;
                                for (j = 0; j < sizeof(vbuf) && len--;
                                    j++, s += k) {
                                        if (*s == '\0')
                                                break;
                                        if (isprint((unsigned char)*s))
                                                vbuf[j] = *s;
                                }
                                if (j == sizeof(vbuf))
                                        --j;
                                vbuf[j] = '\0';
        return 1;
}

private int
cdf_file_summary_info(struct magic_set *ms, const cdf_header_t *h,
    const cdf_stream_t *sst, const cdf_directory_t *root_storage)
{
                        break;
                }
		if (root_storage) {
			str = cdf_clsid_to_mime(root_storage->d_storage_uuid, clsid2desc);
			if (str)
				if (file_printf(ms, ", %s", str) == -1)
					return -2;
			}
		}

        m = cdf_file_property_info(ms, info, count, root_storage);
        free(info);

        return m == -1 ? -2 : m;
}

protected int
file_trycdf(struct magic_set *ms, int fd, const unsigned char *buf,
    size_t nbytes)
{
        cdf_dir_t dir;
        int i;
        const char *expn = "";
        const char *corrupt = "corrupt: ";
        const cdf_directory_t *root_storage;

        info.i_fd = fd;
        info.i_buf = buf;
#ifdef CDF_DEBUG
        cdf_dump_dir(&info, &h, &sat, &ssat, &sst, &dir);
#endif

        if ((i = cdf_read_summary_info(&info, &h, &sat, &ssat, &sst, &dir,
            &scn)) == -1) {
                if (errno == ESRCH) {
                        corrupt = expn;
                        expn = "No summary info";
                } else {
                        expn = "Cannot read summary info";
                }
                goto out4;
        }
#ifdef CDF_DEBUG
        cdf_dump_summary_info(&h, &scn);
#endif
        if ((i = cdf_file_summary_info(ms, &h, &scn, root_storage)) < 0)
			i = 1;
		}
	}
        free(scn.sst_tab);
out4:
        free(sst.sst_tab);
out3:
		    "Composite Document File V2 Document") == -1)
		    return -1;
		if (*expn)
		    if (file_printf(ms, ", %s%s", corrupt, expn) == -1)
			return -1;
	    } else {
		if (file_printf(ms, "application/CDFV2-corrupt") == -1)
		    return -1;
	    }
	    i = 1;
	}