struct dbfs_d2fc_hdr {
	u64	len;		/* Length of d2fc buffer without header */
	u16	version;	/* Version of header */
	char	tod_ext[STORE_CLOCK_EXT_SIZE]; /* TOD clock for d2fc */
	u64	count;		/* Number of VM guests in d2fc buffer */
	char	reserved[30];
} __attribute__ ((packed));

struct dbfs_d2fc {
	struct dbfs_d2fc_hdr	hdr;	/* 64 byte header */
	char			buf[];	/* d2fc buffer */
} __attribute__ ((packed));

static int dbfs_diag2fc_create(void **data, void **data_free_ptr, size_t *size)
{
	struct dbfs_d2fc *d2fc;
	unsigned int count;

	d2fc = diag2fc_store(guest_query, &count, sizeof(d2fc->hdr));
	if (IS_ERR(d2fc))
		return PTR_ERR(d2fc);
	get_tod_clock_ext(d2fc->hdr.tod_ext);
	d2fc->hdr.len = count * sizeof(struct diag2fc_data);
	d2fc->hdr.version = DBFS_D2FC_HDR_VERSION;
	d2fc->hdr.count = count;
	memset(&d2fc->hdr.reserved, 0, sizeof(d2fc->hdr.reserved));
	*data = d2fc;
	*data_free_ptr = d2fc;
	*size = d2fc->hdr.len + sizeof(struct dbfs_d2fc_hdr);
	return 0;
}

static struct hypfs_dbfs_file dbfs_file_2fc = {
	.name		= "diag_2fc",
	.data_create	= dbfs_diag2fc_create,
	.data_free	= diag2fc_free,
};

int hypfs_vm_init(void)
{
	if (!MACHINE_IS_VM)
		return 0;
	if (diag2fc(0, all_guests, NULL) > 0)
		guest_query = all_guests;
	else if (diag2fc(0, local_guest, NULL) > 0)
		guest_query = local_guest;
	else
		return -EACCES;
	return hypfs_dbfs_create_file(&dbfs_file_2fc);
}

void hypfs_vm_exit(void)
{
	if (!MACHINE_IS_VM)
		return;
	hypfs_dbfs_remove_file(&dbfs_file_2fc);
}