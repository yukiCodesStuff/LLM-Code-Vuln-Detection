extern ssize_t proc_uid_map_write(struct file *, const char __user *, size_t, loff_t *);
extern ssize_t proc_gid_map_write(struct file *, const char __user *, size_t, loff_t *);
extern ssize_t proc_projid_map_write(struct file *, const char __user *, size_t, loff_t *);
#else

static inline struct user_namespace *get_user_ns(struct user_namespace *ns)
{
{
}

#endif

#endif /* _LINUX_USER_H */