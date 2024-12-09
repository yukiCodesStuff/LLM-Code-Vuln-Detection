			const struct nls_table *, int);
	struct cifs_ntsd * (*get_acl)(struct cifs_sb_info *, struct inode *,
			const char *, u32 *);
	int (*set_acl)(struct cifs_ntsd *, __u32, struct inode *, const char *,
			int);
};
