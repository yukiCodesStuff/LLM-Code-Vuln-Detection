{
	if (acl && atomic_dec_and_test(&acl->a_refcount))
		kfree_rcu(acl, a_rcu);
}


/* posix_acl.c */

extern void posix_acl_init(struct posix_acl *, int);
extern struct posix_acl *posix_acl_alloc(int, gfp_t);
extern int posix_acl_valid(struct user_namespace *, const struct posix_acl *);
extern int posix_acl_permission(struct inode *, const struct posix_acl *, int);
extern struct posix_acl *posix_acl_from_mode(umode_t, gfp_t);
extern int posix_acl_equiv_mode(const struct posix_acl *, umode_t *);
extern int __posix_acl_create(struct posix_acl **, gfp_t, umode_t *);
extern int __posix_acl_chmod(struct posix_acl **, gfp_t, umode_t);

extern struct posix_acl *get_posix_acl(struct inode *, int);
extern int set_posix_acl(struct inode *, int, struct posix_acl *);

#ifdef CONFIG_FS_POSIX_ACL
extern int posix_acl_chmod(struct inode *, umode_t);
extern int posix_acl_create(struct inode *, umode_t *, struct posix_acl **,
		struct posix_acl **);
extern int posix_acl_update_mode(struct inode *, umode_t *, struct posix_acl **);

extern int simple_set_acl(struct inode *, struct posix_acl *, int);
extern int simple_acl_create(struct inode *, struct inode *);

struct posix_acl *get_cached_acl(struct inode *inode, int type);
struct posix_acl *get_cached_acl_rcu(struct inode *inode, int type);
void set_cached_acl(struct inode *inode, int type, struct posix_acl *acl);
void forget_cached_acl(struct inode *inode, int type);
void forget_all_cached_acls(struct inode *inode);

static inline void cache_no_acl(struct inode *inode)
{
	inode->i_acl = NULL;
	inode->i_default_acl = NULL;
}