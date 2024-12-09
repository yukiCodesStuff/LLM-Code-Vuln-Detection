
extern int cifs_get_inode_info(struct inode **inode, const char *full_path,
			       FILE_ALL_INFO *data, struct super_block *sb,
			       int xid, const __u16 *fid);
extern int cifs_get_inode_info_unix(struct inode **pinode,
			const unsigned char *search_path,
			struct super_block *sb, unsigned int xid);
extern int cifs_set_file_info(struct inode *inode, struct iattr *attrs,
				      const unsigned int xid);
extern int cifs_acl_to_fattr(struct cifs_sb_info *cifs_sb,
			      struct cifs_fattr *fattr, struct inode *inode,
			      const char *path, const __u16 *pfid);
extern int id_mode_to_cifs_acl(struct inode *inode, const char *path, __u64,
					kuid_t, kgid_t);
extern struct cifs_ntsd *get_cifs_acl(struct cifs_sb_info *, struct inode *,
					const char *, u32 *);
extern int set_cifs_acl(struct cifs_ntsd *, __u32, struct inode *,
				const char *, int);

extern void dequeue_mid(struct mid_q_entry *mid, bool malformed);