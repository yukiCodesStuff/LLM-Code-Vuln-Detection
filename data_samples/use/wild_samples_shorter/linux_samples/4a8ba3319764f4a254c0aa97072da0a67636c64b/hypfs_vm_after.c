struct dbfs_d2fc_hdr {
	u64	len;		/* Length of d2fc buffer without header */
	u16	version;	/* Version of header */
	char	tod_ext[STORE_CLOCK_EXT_SIZE]; /* TOD clock for d2fc */
	u64	count;		/* Number of VM guests in d2fc buffer */
	char	reserved[30];
} __attribute__ ((packed));
