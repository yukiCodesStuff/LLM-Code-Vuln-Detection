extern int posix_acl_chmod(struct inode *, umode_t);
extern int posix_acl_create(struct inode *, umode_t *, struct posix_acl **,
		struct posix_acl **);
extern int posix_acl_update_mode(struct inode *, umode_t *, struct posix_acl **);

extern int simple_set_acl(struct inode *, struct posix_acl *, int);
extern int simple_acl_create(struct inode *, struct inode *);
