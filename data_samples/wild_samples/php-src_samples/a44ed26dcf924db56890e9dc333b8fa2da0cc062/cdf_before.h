typedef struct {
	int i_fd;
	const unsigned char *i_buf;
	size_t i_len;
} cdf_info_t;

struct timeval;
int cdf_timestamp_to_timespec(struct timeval *, cdf_timestamp_t);
int cdf_timespec_to_timestamp(cdf_timestamp_t *, const struct timeval *);
int cdf_read_header(const cdf_info_t *, cdf_header_t *);
void cdf_swap_header(cdf_header_t *);
void cdf_unpack_header(cdf_header_t *, char *);
void cdf_swap_dir(cdf_directory_t *);
void cdf_unpack_dir(cdf_directory_t *, char *);
void cdf_swap_class(cdf_classid_t *);
ssize_t cdf_read_sector(const cdf_info_t *, void *, size_t, size_t,
    const cdf_header_t *, cdf_secid_t);
ssize_t cdf_read_short_sector(const cdf_stream_t *, void *, size_t, size_t,
    const cdf_header_t *, cdf_secid_t);
int cdf_read_sat(const cdf_info_t *, cdf_header_t *, cdf_sat_t *);
size_t cdf_count_chain(const cdf_sat_t *, cdf_secid_t, size_t);
int cdf_read_long_sector_chain(const cdf_info_t *, const cdf_header_t *,
    const cdf_sat_t *, cdf_secid_t, size_t, cdf_stream_t *);
int cdf_read_short_sector_chain(const cdf_header_t *, const cdf_sat_t *,
    const cdf_stream_t *, cdf_secid_t, size_t, cdf_stream_t *);
int cdf_read_sector_chain(const cdf_info_t *, const cdf_header_t *,
    const cdf_sat_t *, const cdf_sat_t *, const cdf_stream_t *, cdf_secid_t,
    size_t, cdf_stream_t *);
int cdf_read_dir(const cdf_info_t *, const cdf_header_t *, const cdf_sat_t *,
    cdf_dir_t *);
int cdf_read_ssat(const cdf_info_t *, const cdf_header_t *, const cdf_sat_t *,
    cdf_sat_t *);
int cdf_read_short_stream(const cdf_info_t *, const cdf_header_t *,
    const cdf_sat_t *, const cdf_dir_t *, cdf_stream_t *,
    const cdf_directory_t **);
int cdf_read_property_info(const cdf_stream_t *, const cdf_header_t *, uint32_t,
    cdf_property_info_t **, size_t *, size_t *);
int cdf_read_summary_info(const cdf_info_t *, const cdf_header_t *,
    const cdf_sat_t *, const cdf_sat_t *, const cdf_stream_t *,
    const cdf_dir_t *, cdf_stream_t *);
int cdf_unpack_summary_info(const cdf_stream_t *, const cdf_header_t *,
    cdf_summary_info_header_t *, cdf_property_info_t **, size_t *);
int cdf_print_classid(char *, size_t, const cdf_classid_t *);
int cdf_print_property_name(char *, size_t, uint32_t);
int cdf_print_elapsed_time(char *, size_t, cdf_timestamp_t);
uint16_t cdf_tole2(uint16_t);
uint32_t cdf_tole4(uint32_t);
uint64_t cdf_tole8(uint64_t);
char *cdf_ctime(const time_t *, char *);

#ifdef CDF_DEBUG
void cdf_dump_header(const cdf_header_t *);
void cdf_dump_sat(const char *, const cdf_sat_t *, size_t);
void cdf_dump(void *, size_t);
void cdf_dump_stream(const cdf_header_t *, const cdf_stream_t *);
void cdf_dump_dir(const cdf_info_t *, const cdf_header_t *, const cdf_sat_t *,
    const cdf_sat_t *, const cdf_stream_t *, const cdf_dir_t *);
void cdf_dump_property_info(const cdf_property_info_t *, size_t);
void cdf_dump_summary_info(const cdf_header_t *, const cdf_stream_t *);
#endif


#endif /* _H_CDF_ */
typedef struct {
	int i_fd;
	const unsigned char *i_buf;
	size_t i_len;
} cdf_info_t;

struct timeval;
int cdf_timestamp_to_timespec(struct timeval *, cdf_timestamp_t);
int cdf_timespec_to_timestamp(cdf_timestamp_t *, const struct timeval *);
int cdf_read_header(const cdf_info_t *, cdf_header_t *);
void cdf_swap_header(cdf_header_t *);
void cdf_unpack_header(cdf_header_t *, char *);
void cdf_swap_dir(cdf_directory_t *);
void cdf_unpack_dir(cdf_directory_t *, char *);
void cdf_swap_class(cdf_classid_t *);
ssize_t cdf_read_sector(const cdf_info_t *, void *, size_t, size_t,
    const cdf_header_t *, cdf_secid_t);
ssize_t cdf_read_short_sector(const cdf_stream_t *, void *, size_t, size_t,
    const cdf_header_t *, cdf_secid_t);
int cdf_read_sat(const cdf_info_t *, cdf_header_t *, cdf_sat_t *);
size_t cdf_count_chain(const cdf_sat_t *, cdf_secid_t, size_t);
int cdf_read_long_sector_chain(const cdf_info_t *, const cdf_header_t *,
    const cdf_sat_t *, cdf_secid_t, size_t, cdf_stream_t *);
int cdf_read_short_sector_chain(const cdf_header_t *, const cdf_sat_t *,
    const cdf_stream_t *, cdf_secid_t, size_t, cdf_stream_t *);
int cdf_read_sector_chain(const cdf_info_t *, const cdf_header_t *,
    const cdf_sat_t *, const cdf_sat_t *, const cdf_stream_t *, cdf_secid_t,
    size_t, cdf_stream_t *);
int cdf_read_dir(const cdf_info_t *, const cdf_header_t *, const cdf_sat_t *,
    cdf_dir_t *);
int cdf_read_ssat(const cdf_info_t *, const cdf_header_t *, const cdf_sat_t *,
    cdf_sat_t *);
int cdf_read_short_stream(const cdf_info_t *, const cdf_header_t *,
    const cdf_sat_t *, const cdf_dir_t *, cdf_stream_t *,
    const cdf_directory_t **);
int cdf_read_property_info(const cdf_stream_t *, const cdf_header_t *, uint32_t,
    cdf_property_info_t **, size_t *, size_t *);
int cdf_read_summary_info(const cdf_info_t *, const cdf_header_t *,
    const cdf_sat_t *, const cdf_sat_t *, const cdf_stream_t *,
    const cdf_dir_t *, cdf_stream_t *);
int cdf_unpack_summary_info(const cdf_stream_t *, const cdf_header_t *,
    cdf_summary_info_header_t *, cdf_property_info_t **, size_t *);
int cdf_print_classid(char *, size_t, const cdf_classid_t *);
int cdf_print_property_name(char *, size_t, uint32_t);
int cdf_print_elapsed_time(char *, size_t, cdf_timestamp_t);
uint16_t cdf_tole2(uint16_t);
uint32_t cdf_tole4(uint32_t);
uint64_t cdf_tole8(uint64_t);
char *cdf_ctime(const time_t *, char *);

#ifdef CDF_DEBUG
void cdf_dump_header(const cdf_header_t *);
void cdf_dump_sat(const char *, const cdf_sat_t *, size_t);
void cdf_dump(void *, size_t);
void cdf_dump_stream(const cdf_header_t *, const cdf_stream_t *);
void cdf_dump_dir(const cdf_info_t *, const cdf_header_t *, const cdf_sat_t *,
    const cdf_sat_t *, const cdf_stream_t *, const cdf_dir_t *);
void cdf_dump_property_info(const cdf_property_info_t *, size_t);
void cdf_dump_summary_info(const cdf_header_t *, const cdf_stream_t *);
#endif


#endif /* _H_CDF_ */