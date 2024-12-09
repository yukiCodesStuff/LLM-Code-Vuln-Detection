struct pstore_read_data {
	u64 *id;
	enum pstore_type_id *type;
	int *count;
	struct timespec *timespec;
	bool *compressed;
	char **buf;
};

static inline u64 generic_id(unsigned long timestamp,
			     unsigned int part, int count)
{
	return ((u64) timestamp * 100 + part) * 1000 + count;
}