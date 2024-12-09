	size_t i_len;
} cdf_info_t;


typedef struct {
	uint16_t ce_namlen;
	uint32_t ce_num;
	uint64_t ce_timestamp; 
	uint16_t ce_name[256];
} cdf_catalog_entry_t;

typedef struct {
	size_t cat_num;
	cdf_catalog_entry_t cat_e[0];
} cdf_catalog_t;

struct timeval;
int cdf_timestamp_to_timespec(struct timeval *, cdf_timestamp_t);
int cdf_timespec_to_timestamp(cdf_timestamp_t *, const struct timeval *);
int cdf_read_header(const cdf_info_t *, cdf_header_t *);
    const cdf_directory_t **);
int cdf_read_property_info(const cdf_stream_t *, const cdf_header_t *, uint32_t,
    cdf_property_info_t **, size_t *, size_t *);
int cdf_read_user_stream(const cdf_info_t *, const cdf_header_t *,
    const cdf_sat_t *, const cdf_sat_t *, const cdf_stream_t *,
    const cdf_dir_t *, const char *, cdf_stream_t *);
int cdf_find_stream(const cdf_dir_t *, const char *, int);
int cdf_read_summary_info(const cdf_info_t *, const cdf_header_t *,
    const cdf_sat_t *, const cdf_sat_t *, const cdf_stream_t *,
    const cdf_dir_t *, cdf_stream_t *);
int cdf_unpack_summary_info(const cdf_stream_t *, const cdf_header_t *,
    cdf_summary_info_header_t *, cdf_property_info_t **, size_t *);
int cdf_unpack_catalog(const cdf_header_t *, const cdf_stream_t *,
    cdf_catalog_t **);
int cdf_print_classid(char *, size_t, const cdf_classid_t *);
int cdf_print_property_name(char *, size_t, uint32_t);
int cdf_print_elapsed_time(char *, size_t, cdf_timestamp_t);
uint16_t cdf_tole2(uint16_t);
uint32_t cdf_tole4(uint32_t);
uint64_t cdf_tole8(uint64_t);
char *cdf_ctime(const time_t *, char *);
char *cdf_u16tos8(char *, size_t, const uint16_t *);

#ifdef CDF_DEBUG
void cdf_dump_header(const cdf_header_t *);
void cdf_dump_sat(const char *, const cdf_sat_t *, size_t);
void cdf_dump(const void *, size_t);
void cdf_dump_stream(const cdf_header_t *, const cdf_stream_t *);
void cdf_dump_dir(const cdf_info_t *, const cdf_header_t *, const cdf_sat_t *,
    const cdf_sat_t *, const cdf_stream_t *, const cdf_dir_t *);
void cdf_dump_property_info(const cdf_property_info_t *, size_t);
void cdf_dump_summary_info(const cdf_header_t *, const cdf_stream_t *);
void cdf_dump_catalog(const cdf_header_t *, const cdf_stream_t *);
#endif


#endif /* _H_CDF_ */