enum lockdown_reason {
	LOCKDOWN_NONE,
	LOCKDOWN_MODULE_SIGNATURE,
	LOCKDOWN_DEV_MEM,
	LOCKDOWN_EFI_TEST,
	LOCKDOWN_KEXEC,
	LOCKDOWN_HIBERNATION,
	LOCKDOWN_PCI_ACCESS,
	LOCKDOWN_IOPORT,
	LOCKDOWN_MSR,
	LOCKDOWN_ACPI_TABLES,
	LOCKDOWN_PCMCIA_CIS,
	LOCKDOWN_TIOCSSERIAL,
	LOCKDOWN_MODULE_PARAMETERS,
	LOCKDOWN_MMIOTRACE,
	LOCKDOWN_DEBUGFS,
	LOCKDOWN_XMON_WR,
	LOCKDOWN_BPF_WRITE_USER,
	LOCKDOWN_INTEGRITY_MAX,
	LOCKDOWN_KCORE,
	LOCKDOWN_KPROBES,
	LOCKDOWN_BPF_READ_KERNEL,
	LOCKDOWN_PERF,
	LOCKDOWN_TRACEFS,
	LOCKDOWN_XMON_RW,
	LOCKDOWN_XFRM_SECRET,
	LOCKDOWN_CONFIDENTIALITY_MAX,
};

extern const char *const lockdown_reasons[LOCKDOWN_CONFIDENTIALITY_MAX+1];

/* These functions are in security/commoncap.c */
extern int cap_capable(const struct cred *cred, struct user_namespace *ns,
		       int cap, unsigned int opts);
extern int cap_settime(const struct timespec64 *ts, const struct timezone *tz);
extern int cap_ptrace_access_check(struct task_struct *child, unsigned int mode);
extern int cap_ptrace_traceme(struct task_struct *parent);
extern int cap_capget(struct task_struct *target, kernel_cap_t *effective, kernel_cap_t *inheritable, kernel_cap_t *permitted);
extern int cap_capset(struct cred *new, const struct cred *old,
		      const kernel_cap_t *effective,
		      const kernel_cap_t *inheritable,
		      const kernel_cap_t *permitted);
extern int cap_bprm_creds_from_file(struct linux_binprm *bprm, struct file *file);
int cap_inode_setxattr(struct dentry *dentry, const char *name,
		       const void *value, size_t size, int flags);
int cap_inode_removexattr(struct user_namespace *mnt_userns,
			  struct dentry *dentry, const char *name);
int cap_inode_need_killpriv(struct dentry *dentry);
int cap_inode_killpriv(struct user_namespace *mnt_userns,
		       struct dentry *dentry);
int cap_inode_getsecurity(struct user_namespace *mnt_userns,
			  struct inode *inode, const char *name, void **buffer,
			  bool alloc);
extern int cap_mmap_addr(unsigned long addr);
extern int cap_mmap_file(struct file *file, unsigned long reqprot,
			 unsigned long prot, unsigned long flags);
extern int cap_task_fix_setuid(struct cred *new, const struct cred *old, int flags);
extern int cap_task_prctl(int option, unsigned long arg2, unsigned long arg3,
			  unsigned long arg4, unsigned long arg5);
extern int cap_task_setscheduler(struct task_struct *p);
extern int cap_task_setioprio(struct task_struct *p, int ioprio);
extern int cap_task_setnice(struct task_struct *p, int nice);
extern int cap_vm_enough_memory(struct mm_struct *mm, long pages);

struct msghdr;
struct sk_buff;
struct sock;
struct sockaddr;
struct socket;
struct flowi_common;
struct dst_entry;
struct xfrm_selector;
struct xfrm_policy;
struct xfrm_state;
struct xfrm_user_sec_ctx;
struct seq_file;
struct sctp_association;

#ifdef CONFIG_MMU
extern unsigned long mmap_min_addr;
extern unsigned long dac_mmap_min_addr;
#else
#define mmap_min_addr		0UL
#define dac_mmap_min_addr	0UL
#endif

/*
 * Values used in the task_security_ops calls
 */
/* setuid or setgid, id0 == uid or gid */
#define LSM_SETID_ID	1

/* setreuid or setregid, id0 == real, id1 == eff */
#define LSM_SETID_RE	2

/* setresuid or setresgid, id0 == real, id1 == eff, uid2 == saved */
#define LSM_SETID_RES	4

/* setfsuid or setfsgid, id0 == fsuid or fsgid */
#define LSM_SETID_FS	8

/* Flags for security_task_prlimit(). */
#define LSM_PRLIMIT_READ  1
#define LSM_PRLIMIT_WRITE 2

/* forward declares to avoid warnings */
struct sched_param;
struct request_sock;

/* bprm->unsafe reasons */
#define LSM_UNSAFE_SHARE	1
#define LSM_UNSAFE_PTRACE	2
#define LSM_UNSAFE_NO_NEW_PRIVS	4

#ifdef CONFIG_MMU
extern int mmap_min_addr_handler(struct ctl_table *table, int write,
				 void *buffer, size_t *lenp, loff_t *ppos);
#endif

/* security_inode_init_security callback function to write xattrs */
typedef int (*initxattrs) (struct inode *inode,
			   const struct xattr *xattr_array, void *fs_data);


/* Keep the kernel_load_data_id enum in sync with kernel_read_file_id */
#define __data_id_enumify(ENUM, dummy) LOADING_ ## ENUM,
#define __data_id_stringify(dummy, str) #str,

enum kernel_load_data_id {
	__kernel_read_file_id(__data_id_enumify)
};

static const char * const kernel_load_data_str[] = {
	__kernel_read_file_id(__data_id_stringify)
};

static inline const char *kernel_load_data_id_str(enum kernel_load_data_id id)
{
	if ((unsigned)id >= LOADING_MAX_ID)
		return kernel_load_data_str[LOADING_UNKNOWN];

	return kernel_load_data_str[id];
}

#ifdef CONFIG_SECURITY

int call_blocking_lsm_notifier(enum lsm_event event, void *data);
int register_blocking_lsm_notifier(struct notifier_block *nb);
int unregister_blocking_lsm_notifier(struct notifier_block *nb);

/* prototypes */
extern int security_init(void);
extern int early_security_init(void);

/* Security operations */
int security_binder_set_context_mgr(const struct cred *mgr);
int security_binder_transaction(const struct cred *from,
				const struct cred *to);
int security_binder_transfer_binder(const struct cred *from,
				    const struct cred *to);
int security_binder_transfer_file(const struct cred *from,
				  const struct cred *to, struct file *file);
int security_ptrace_access_check(struct task_struct *child, unsigned int mode);
int security_ptrace_traceme(struct task_struct *parent);
int security_capget(struct task_struct *target,
		    kernel_cap_t *effective,
		    kernel_cap_t *inheritable,
		    kernel_cap_t *permitted);
int security_capset(struct cred *new, const struct cred *old,
		    const kernel_cap_t *effective,
		    const kernel_cap_t *inheritable,
		    const kernel_cap_t *permitted);
int security_capable(const struct cred *cred,
		       struct user_namespace *ns,
		       int cap,
		       unsigned int opts);
int security_quotactl(int cmds, int type, int id, struct super_block *sb);
int security_quota_on(struct dentry *dentry);
int security_syslog(int type);
int security_settime64(const struct timespec64 *ts, const struct timezone *tz);
int security_vm_enough_memory_mm(struct mm_struct *mm, long pages);
int security_bprm_creds_for_exec(struct linux_binprm *bprm);
int security_bprm_creds_from_file(struct linux_binprm *bprm, struct file *file);
int security_bprm_check(struct linux_binprm *bprm);
void security_bprm_committing_creds(struct linux_binprm *bprm);
void security_bprm_committed_creds(struct linux_binprm *bprm);
int security_fs_context_dup(struct fs_context *fc, struct fs_context *src_fc);
int security_fs_context_parse_param(struct fs_context *fc, struct fs_parameter *param);
int security_sb_alloc(struct super_block *sb);
void security_sb_delete(struct super_block *sb);
void security_sb_free(struct super_block *sb);
void security_free_mnt_opts(void **mnt_opts);
int security_sb_eat_lsm_opts(char *options, void **mnt_opts);
int security_sb_mnt_opts_compat(struct super_block *sb, void *mnt_opts);
int security_sb_remount(struct super_block *sb, void *mnt_opts);
int security_sb_kern_mount(struct super_block *sb);
int security_sb_show_options(struct seq_file *m, struct super_block *sb);
int security_sb_statfs(struct dentry *dentry);
int security_sb_mount(const char *dev_name, const struct path *path,
		      const char *type, unsigned long flags, void *data);
int security_sb_umount(struct vfsmount *mnt, int flags);
int security_sb_pivotroot(const struct path *old_path, const struct path *new_path);
int security_sb_set_mnt_opts(struct super_block *sb,
				void *mnt_opts,
				unsigned long kern_flags,
				unsigned long *set_kern_flags);
int security_sb_clone_mnt_opts(const struct super_block *oldsb,
				struct super_block *newsb,
				unsigned long kern_flags,
				unsigned long *set_kern_flags);
int security_move_mount(const struct path *from_path, const struct path *to_path);
int security_dentry_init_security(struct dentry *dentry, int mode,
				  const struct qstr *name,
				  const char **xattr_name, void **ctx,
				  u32 *ctxlen);
int security_dentry_create_files_as(struct dentry *dentry, int mode,
					struct qstr *name,
					const struct cred *old,
					struct cred *new);
int security_path_notify(const struct path *path, u64 mask,
					unsigned int obj_type);
int security_inode_alloc(struct inode *inode);
void security_inode_free(struct inode *inode);
int security_inode_init_security(struct inode *inode, struct inode *dir,
				 const struct qstr *qstr,
				 initxattrs initxattrs, void *fs_data);
int security_inode_init_security_anon(struct inode *inode,
				      const struct qstr *name,
				      const struct inode *context_inode);
int security_old_inode_init_security(struct inode *inode, struct inode *dir,
				     const struct qstr *qstr, const char **name,
				     void **value, size_t *len);
int security_inode_create(struct inode *dir, struct dentry *dentry, umode_t mode);
int security_inode_link(struct dentry *old_dentry, struct inode *dir,
			 struct dentry *new_dentry);
int security_inode_unlink(struct inode *dir, struct dentry *dentry);
int security_inode_symlink(struct inode *dir, struct dentry *dentry,
			   const char *old_name);
int security_inode_mkdir(struct inode *dir, struct dentry *dentry, umode_t mode);
int security_inode_rmdir(struct inode *dir, struct dentry *dentry);
int security_inode_mknod(struct inode *dir, struct dentry *dentry, umode_t mode, dev_t dev);
int security_inode_rename(struct inode *old_dir, struct dentry *old_dentry,
			  struct inode *new_dir, struct dentry *new_dentry,
			  unsigned int flags);
int security_inode_readlink(struct dentry *dentry);
int security_inode_follow_link(struct dentry *dentry, struct inode *inode,
			       bool rcu);
int security_inode_permission(struct inode *inode, int mask);
int security_inode_setattr(struct dentry *dentry, struct iattr *attr);
int security_inode_getattr(const struct path *path);
int security_inode_setxattr(struct user_namespace *mnt_userns,
			    struct dentry *dentry, const char *name,
			    const void *value, size_t size, int flags);
void security_inode_post_setxattr(struct dentry *dentry, const char *name,
				  const void *value, size_t size, int flags);
int security_inode_getxattr(struct dentry *dentry, const char *name);
int security_inode_listxattr(struct dentry *dentry);
int security_inode_removexattr(struct user_namespace *mnt_userns,
			       struct dentry *dentry, const char *name);
int security_inode_need_killpriv(struct dentry *dentry);
int security_inode_killpriv(struct user_namespace *mnt_userns,
			    struct dentry *dentry);
int security_inode_getsecurity(struct user_namespace *mnt_userns,
			       struct inode *inode, const char *name,
			       void **buffer, bool alloc);
int security_inode_setsecurity(struct inode *inode, const char *name, const void *value, size_t size, int flags);
int security_inode_listsecurity(struct inode *inode, char *buffer, size_t buffer_size);
void security_inode_getsecid(struct inode *inode, u32 *secid);
int security_inode_copy_up(struct dentry *src, struct cred **new);
int security_inode_copy_up_xattr(const char *name);
int security_kernfs_init_security(struct kernfs_node *kn_dir,
				  struct kernfs_node *kn);
int security_file_permission(struct file *file, int mask);
int security_file_alloc(struct file *file);
void security_file_free(struct file *file);
int security_file_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
int security_mmap_file(struct file *file, unsigned long prot,
			unsigned long flags);
int security_mmap_addr(unsigned long addr);
int security_file_mprotect(struct vm_area_struct *vma, unsigned long reqprot,
			   unsigned long prot);
int security_file_lock(struct file *file, unsigned int cmd);
int security_file_fcntl(struct file *file, unsigned int cmd, unsigned long arg);
void security_file_set_fowner(struct file *file);
int security_file_send_sigiotask(struct task_struct *tsk,
				 struct fown_struct *fown, int sig);
int security_file_receive(struct file *file);
int security_file_open(struct file *file);
int security_task_alloc(struct task_struct *task, unsigned long clone_flags);
void security_task_free(struct task_struct *task);
int security_cred_alloc_blank(struct cred *cred, gfp_t gfp);
void security_cred_free(struct cred *cred);
int security_prepare_creds(struct cred *new, const struct cred *old, gfp_t gfp);
void security_transfer_creds(struct cred *new, const struct cred *old);
void security_cred_getsecid(const struct cred *c, u32 *secid);
int security_kernel_act_as(struct cred *new, u32 secid);
int security_kernel_create_files_as(struct cred *new, struct inode *inode);
int security_kernel_module_request(char *kmod_name);
int security_kernel_load_data(enum kernel_load_data_id id, bool contents);
int security_kernel_post_load_data(char *buf, loff_t size,
				   enum kernel_load_data_id id,
				   char *description);
int security_kernel_read_file(struct file *file, enum kernel_read_file_id id,
			      bool contents);
int security_kernel_post_read_file(struct file *file, char *buf, loff_t size,
				   enum kernel_read_file_id id);
int security_task_fix_setuid(struct cred *new, const struct cred *old,
			     int flags);
int security_task_fix_setgid(struct cred *new, const struct cred *old,
			     int flags);
int security_task_setpgid(struct task_struct *p, pid_t pgid);
int security_task_getpgid(struct task_struct *p);
int security_task_getsid(struct task_struct *p);
void security_current_getsecid_subj(u32 *secid);
void security_task_getsecid_obj(struct task_struct *p, u32 *secid);
int security_task_setnice(struct task_struct *p, int nice);
int security_task_setioprio(struct task_struct *p, int ioprio);
int security_task_getioprio(struct task_struct *p);
int security_task_prlimit(const struct cred *cred, const struct cred *tcred,
			  unsigned int flags);
int security_task_setrlimit(struct task_struct *p, unsigned int resource,
		struct rlimit *new_rlim);
int security_task_setscheduler(struct task_struct *p);
int security_task_getscheduler(struct task_struct *p);
int security_task_movememory(struct task_struct *p);
int security_task_kill(struct task_struct *p, struct kernel_siginfo *info,
			int sig, const struct cred *cred);
int security_task_prctl(int option, unsigned long arg2, unsigned long arg3,
			unsigned long arg4, unsigned long arg5);
void security_task_to_inode(struct task_struct *p, struct inode *inode);
int security_ipc_permission(struct kern_ipc_perm *ipcp, short flag);
void security_ipc_getsecid(struct kern_ipc_perm *ipcp, u32 *secid);
int security_msg_msg_alloc(struct msg_msg *msg);
void security_msg_msg_free(struct msg_msg *msg);
int security_msg_queue_alloc(struct kern_ipc_perm *msq);
void security_msg_queue_free(struct kern_ipc_perm *msq);
int security_msg_queue_associate(struct kern_ipc_perm *msq, int msqflg);
int security_msg_queue_msgctl(struct kern_ipc_perm *msq, int cmd);
int security_msg_queue_msgsnd(struct kern_ipc_perm *msq,
			      struct msg_msg *msg, int msqflg);
int security_msg_queue_msgrcv(struct kern_ipc_perm *msq, struct msg_msg *msg,
			      struct task_struct *target, long type, int mode);
int security_shm_alloc(struct kern_ipc_perm *shp);
void security_shm_free(struct kern_ipc_perm *shp);
int security_shm_associate(struct kern_ipc_perm *shp, int shmflg);
int security_shm_shmctl(struct kern_ipc_perm *shp, int cmd);
int security_shm_shmat(struct kern_ipc_perm *shp, char __user *shmaddr, int shmflg);
int security_sem_alloc(struct kern_ipc_perm *sma);
void security_sem_free(struct kern_ipc_perm *sma);
int security_sem_associate(struct kern_ipc_perm *sma, int semflg);
int security_sem_semctl(struct kern_ipc_perm *sma, int cmd);
int security_sem_semop(struct kern_ipc_perm *sma, struct sembuf *sops,
			unsigned nsops, int alter);
void security_d_instantiate(struct dentry *dentry, struct inode *inode);
int security_getprocattr(struct task_struct *p, const char *lsm, char *name,
			 char **value);
int security_setprocattr(const char *lsm, const char *name, void *value,
			 size_t size);
int security_netlink_send(struct sock *sk, struct sk_buff *skb);
int security_ismaclabel(const char *name);
int security_secid_to_secctx(u32 secid, char **secdata, u32 *seclen);
int security_secctx_to_secid(const char *secdata, u32 seclen, u32 *secid);
void security_release_secctx(char *secdata, u32 seclen);
void security_inode_invalidate_secctx(struct inode *inode);
int security_inode_notifysecctx(struct inode *inode, void *ctx, u32 ctxlen);
int security_inode_setsecctx(struct dentry *dentry, void *ctx, u32 ctxlen);
int security_inode_getsecctx(struct inode *inode, void **ctx, u32 *ctxlen);
int security_locked_down(enum lockdown_reason what);
#else /* CONFIG_SECURITY */

static inline int call_blocking_lsm_notifier(enum lsm_event event, void *data)
{
	return 0;
}

static inline int register_blocking_lsm_notifier(struct notifier_block *nb)
{
	return 0;
}

static inline  int unregister_blocking_lsm_notifier(struct notifier_block *nb)
{
	return 0;
}

static inline void security_free_mnt_opts(void **mnt_opts)
{
}

/*
 * This is the default capabilities functionality.  Most of these functions
 * are just stubbed out, but a few must call the proper capable code.
 */

static inline int security_init(void)
{
	return 0;
}

static inline int early_security_init(void)
{
	return 0;
}

static inline int security_binder_set_context_mgr(const struct cred *mgr)
{
	return 0;
}

static inline int security_binder_transaction(const struct cred *from,
					      const struct cred *to)
{
	return 0;
}

static inline int security_binder_transfer_binder(const struct cred *from,
						  const struct cred *to)
{
	return 0;
}

static inline int security_binder_transfer_file(const struct cred *from,
						const struct cred *to,
						struct file *file)
{
	return 0;
}

static inline int security_ptrace_access_check(struct task_struct *child,
					     unsigned int mode)
{
	return cap_ptrace_access_check(child, mode);
}

static inline int security_ptrace_traceme(struct task_struct *parent)
{
	return cap_ptrace_traceme(parent);
}

static inline int security_capget(struct task_struct *target,
				   kernel_cap_t *effective,
				   kernel_cap_t *inheritable,
				   kernel_cap_t *permitted)
{
	return cap_capget(target, effective, inheritable, permitted);
}

static inline int security_capset(struct cred *new,
				   const struct cred *old,
				   const kernel_cap_t *effective,
				   const kernel_cap_t *inheritable,
				   const kernel_cap_t *permitted)
{
	return cap_capset(new, old, effective, inheritable, permitted);
}

static inline int security_capable(const struct cred *cred,
				   struct user_namespace *ns,
				   int cap,
				   unsigned int opts)
{
	return cap_capable(cred, ns, cap, opts);
}

static inline int security_quotactl(int cmds, int type, int id,
				     struct super_block *sb)
{
	return 0;
}

static inline int security_quota_on(struct dentry *dentry)
{
	return 0;
}

static inline int security_syslog(int type)
{
	return 0;
}

static inline int security_settime64(const struct timespec64 *ts,
				     const struct timezone *tz)
{
	return cap_settime(ts, tz);
}

static inline int security_vm_enough_memory_mm(struct mm_struct *mm, long pages)
{
	return __vm_enough_memory(mm, pages, cap_vm_enough_memory(mm, pages));
}

static inline int security_bprm_creds_for_exec(struct linux_binprm *bprm)
{
	return 0;
}

static inline int security_bprm_creds_from_file(struct linux_binprm *bprm,
						struct file *file)
{
	return cap_bprm_creds_from_file(bprm, file);
}

static inline int security_bprm_check(struct linux_binprm *bprm)
{
	return 0;
}

static inline void security_bprm_committing_creds(struct linux_binprm *bprm)
{
}

static inline void security_bprm_committed_creds(struct linux_binprm *bprm)
{
}

static inline int security_fs_context_dup(struct fs_context *fc,
					  struct fs_context *src_fc)
{
	return 0;
}
static inline int security_fs_context_parse_param(struct fs_context *fc,
						  struct fs_parameter *param)
{
	return -ENOPARAM;
}

static inline int security_sb_alloc(struct super_block *sb)
{
	return 0;
}

static inline void security_sb_delete(struct super_block *sb)
{ }

static inline void security_sb_free(struct super_block *sb)
{ }