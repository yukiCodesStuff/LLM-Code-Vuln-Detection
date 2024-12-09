extern struct vfsmount *mntget(struct vfsmount *mnt);
extern struct vfsmount *mnt_clone_internal(struct path *path);
extern int __mnt_is_readonly(struct vfsmount *mnt);
extern bool mnt_may_suid(struct vfsmount *mnt);

struct path;
extern struct vfsmount *clone_private_mount(struct path *path);
