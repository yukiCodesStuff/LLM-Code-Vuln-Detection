
extern struct file *proc_ns_fget(int fd);
#define get_proc_ns(inode) ((struct ns_common *)(inode)->i_private)
extern int ns_get_path(struct path *path, struct task_struct *task,
			const struct proc_ns_operations *ns_ops);
typedef struct ns_common *ns_get_path_helper_t(void *);
extern int ns_get_path_cb(struct path *path, ns_get_path_helper_t ns_get_cb,
			    void *private_data);

extern int ns_get_name(char *buf, size_t size, struct task_struct *task,
			const struct proc_ns_operations *ns_ops);