extern ssize_t proc_uid_map_write(struct file *, const char __user *, size_t, loff_t *);
extern ssize_t proc_gid_map_write(struct file *, const char __user *, size_t, loff_t *);
extern ssize_t proc_projid_map_write(struct file *, const char __user *, size_t, loff_t *);
extern bool userns_may_setgroups(const struct user_namespace *ns);
#else

static inline struct user_namespace *get_user_ns(struct user_namespace *ns)
{
{
}

static inline bool userns_may_setgroups(const struct user_namespace *ns)
{
	return true;
}
#endif

#endif /* _LINUX_USER_H */