static inline u64 generic_id(unsigned long timestamp,
			     unsigned int part, int count)
{
	return ((u64) timestamp * 100 + part) * 1000 + count;
}

static int efi_pstore_read_func(struct efivar_entry *entry, void *data)
{