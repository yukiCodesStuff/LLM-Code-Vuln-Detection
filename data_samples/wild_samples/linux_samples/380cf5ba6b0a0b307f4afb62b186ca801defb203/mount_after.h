struct vfsmount {
	struct dentry *mnt_root;	/* root of the mounted tree */
	struct super_block *mnt_sb;	/* pointer to superblock */
	int mnt_flags;
};

struct file; /* forward dec */
struct path;

extern int mnt_want_write(struct vfsmount *mnt);
extern int mnt_want_write_file(struct file *file);
extern int mnt_clone_write(struct vfsmount *mnt);
extern void mnt_drop_write(struct vfsmount *mnt);
extern void mnt_drop_write_file(struct file *file);
extern void mntput(struct vfsmount *mnt);
extern struct vfsmount *mntget(struct vfsmount *mnt);
extern struct vfsmount *mnt_clone_internal(struct path *path);
extern int __mnt_is_readonly(struct vfsmount *mnt);
extern bool mnt_may_suid(struct vfsmount *mnt);

struct path;
extern struct vfsmount *clone_private_mount(struct path *path);

struct file_system_type;
extern struct vfsmount *vfs_kern_mount(struct file_system_type *type,
				      int flags, const char *name,
				      void *data);

extern void mnt_set_expiry(struct vfsmount *mnt, struct list_head *expiry_list);
extern void mark_mounts_for_expiry(struct list_head *mounts);

extern dev_t name_to_dev_t(const char *name);

#endif /* _LINUX_MOUNT_H */