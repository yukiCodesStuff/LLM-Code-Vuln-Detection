{
	struct path path;
	int error = -EACCES;

	if (!dentry)
		return ERR_PTR(-ECHILD);

	/* Are we allowed to snoop on the tasks file descriptors? */
	if (!proc_fd_access_allowed(inode))
		goto out;

	error = PROC_I(inode)->op.proc_get_link(dentry, &path);
	if (error)
		goto out;

	error = nd_jump_link(&path);
out:
	return ERR_PTR(error);
}

static int do_proc_readlink(struct path *path, char __user *buffer, int buflen)
{
	char *tmp = (char *)__get_free_page(GFP_KERNEL);
	char *pathname;
	int len;

	if (!tmp)
		return -ENOMEM;

	pathname = d_path(path, tmp, PAGE_SIZE);
	len = PTR_ERR(pathname);
	if (IS_ERR(pathname))
		goto out;
	len = tmp + PAGE_SIZE - 1 - pathname;

	if (len > buflen)
		len = buflen;
	if (copy_to_user(buffer, pathname, len))
		len = -EFAULT;
 out:
	free_page((unsigned long)tmp);
	return len;
}

static int proc_pid_readlink(struct dentry * dentry, char __user * buffer, int buflen)
{
	int error = -EACCES;
	struct inode *inode = d_inode(dentry);
	struct path path;

	/* Are we allowed to snoop on the tasks file descriptors? */
	if (!proc_fd_access_allowed(inode))
		goto out;

	error = PROC_I(inode)->op.proc_get_link(dentry, &path);
	if (error)
		goto out;

	error = do_proc_readlink(&path, buffer, buflen);
	path_put(&path);
out:
	return error;
}

const struct inode_operations proc_pid_link_inode_operations = {
	.readlink	= proc_pid_readlink,
	.get_link	= proc_pid_get_link,
	.setattr	= proc_setattr,
};


/* building an inode */

void task_dump_owner(struct task_struct *task, umode_t mode,
		     kuid_t *ruid, kgid_t *rgid)
{
	/* Depending on the state of dumpable compute who should own a
	 * proc file for a task.
	 */
	const struct cred *cred;
	kuid_t uid;
	kgid_t gid;

	if (unlikely(task->flags & PF_KTHREAD)) {
		*ruid = GLOBAL_ROOT_UID;
		*rgid = GLOBAL_ROOT_GID;
		return;
	}

	/* Default to the tasks effective ownership */
	rcu_read_lock();
	cred = __task_cred(task);
	uid = cred->euid;
	gid = cred->egid;
	rcu_read_unlock();

	/*
	 * Before the /proc/pid/status file was created the only way to read
	 * the effective uid of a /process was to stat /proc/pid.  Reading
	 * /proc/pid/status is slow enough that procps and other packages
	 * kept stating /proc/pid.  To keep the rules in /proc simple I have
	 * made this apply to all per process world readable and executable
	 * directories.
	 */
	if (mode != (S_IFDIR|S_IRUGO|S_IXUGO)) {
		struct mm_struct *mm;
		task_lock(task);
		mm = task->mm;
		/* Make non-dumpable tasks owned by some root */
		if (mm) {
			if (get_dumpable(mm) != SUID_DUMP_USER) {
				struct user_namespace *user_ns = mm->user_ns;

				uid = make_kuid(user_ns, 0);
				if (!uid_valid(uid))
					uid = GLOBAL_ROOT_UID;

				gid = make_kgid(user_ns, 0);
				if (!gid_valid(gid))
					gid = GLOBAL_ROOT_GID;
			}
		} else {
			uid = GLOBAL_ROOT_UID;
			gid = GLOBAL_ROOT_GID;
		}
		task_unlock(task);
	}
	*ruid = uid;
	*rgid = gid;
}

struct inode *proc_pid_make_inode(struct super_block * sb,
				  struct task_struct *task, umode_t mode)
{
	struct inode * inode;
	struct proc_inode *ei;

	/* We need a new inode */

	inode = new_inode(sb);
	if (!inode)
		goto out;

	/* Common stuff */
	ei = PROC_I(inode);
	inode->i_mode = mode;
	inode->i_ino = get_next_ino();
	inode->i_mtime = inode->i_atime = inode->i_ctime = current_time(inode);
	inode->i_op = &proc_def_inode_operations;

	/*
	 * grab the reference to task.
	 */
	ei->pid = get_task_pid(task, PIDTYPE_PID);
	if (!ei->pid)
		goto out_unlock;

	task_dump_owner(task, 0, &inode->i_uid, &inode->i_gid);
	security_task_to_inode(task, inode);

out:
	return inode;

out_unlock:
	iput(inode);
	return NULL;
}

int pid_getattr(const struct path *path, struct kstat *stat,
		u32 request_mask, unsigned int query_flags)
{
	struct inode *inode = d_inode(path->dentry);
	struct pid_namespace *pid = proc_pid_ns(inode);
	struct task_struct *task;

	generic_fillattr(inode, stat);

	stat->uid = GLOBAL_ROOT_UID;
	stat->gid = GLOBAL_ROOT_GID;
	rcu_read_lock();
	task = pid_task(proc_pid(inode), PIDTYPE_PID);
	if (task) {
		if (!has_pid_permissions(pid, task, HIDEPID_INVISIBLE)) {
			rcu_read_unlock();
			/*
			 * This doesn't prevent learning whether PID exists,
			 * it only makes getattr() consistent with readdir().
			 */
			return -ENOENT;
		}
		task_dump_owner(task, inode->i_mode, &stat->uid, &stat->gid);
	}
	rcu_read_unlock();
	return 0;
}

/* dentry stuff */

/*
 * Set <pid>/... inode ownership (can change due to setuid(), etc.)
 */
void pid_update_inode(struct task_struct *task, struct inode *inode)
{
	task_dump_owner(task, inode->i_mode, &inode->i_uid, &inode->i_gid);

	inode->i_mode &= ~(S_ISUID | S_ISGID);
	security_task_to_inode(task, inode);
}

/*
 * Rewrite the inode's ownerships here because the owning task may have
 * performed a setuid(), etc.
 *
 */
static int pid_revalidate(struct dentry *dentry, unsigned int flags)
{
	struct inode *inode;
	struct task_struct *task;

	if (flags & LOOKUP_RCU)
		return -ECHILD;

	inode = d_inode(dentry);
	task = get_proc_task(inode);

	if (task) {
		pid_update_inode(task, inode);
		put_task_struct(task);
		return 1;
	}
	return 0;
}

static inline bool proc_inode_is_dead(struct inode *inode)
{
	return !proc_pid(inode)->tasks[PIDTYPE_PID].first;
}

int pid_delete_dentry(const struct dentry *dentry)
{
	/* Is the task we represent dead?
	 * If so, then don't put the dentry on the lru list,
	 * kill it immediately.
	 */
	return proc_inode_is_dead(d_inode(dentry));
}

const struct dentry_operations pid_dentry_operations =
{
	.d_revalidate	= pid_revalidate,
	.d_delete	= pid_delete_dentry,
};

/* Lookups */

/*
 * Fill a directory entry.
 *
 * If possible create the dcache entry and derive our inode number and
 * file type from dcache entry.
 *
 * Since all of the proc inode numbers are dynamically generated, the inode
 * numbers do not exist until the inode is cache.  This means creating the
 * the dcache entry in readdir is necessary to keep the inode numbers
 * reported by readdir in sync with the inode numbers reported
 * by stat.
 */
bool proc_fill_cache(struct file *file, struct dir_context *ctx,
	const char *name, unsigned int len,
	instantiate_t instantiate, struct task_struct *task, const void *ptr)
{
	struct dentry *child, *dir = file->f_path.dentry;
	struct qstr qname = QSTR_INIT(name, len);
	struct inode *inode;
	unsigned type = DT_UNKNOWN;
	ino_t ino = 1;

	child = d_hash_and_lookup(dir, &qname);
	if (!child) {
		DECLARE_WAIT_QUEUE_HEAD_ONSTACK(wq);
		child = d_alloc_parallel(dir, &qname, &wq);
		if (IS_ERR(child))
			goto end_instantiate;
		if (d_in_lookup(child)) {
			struct dentry *res;
			res = instantiate(child, task, ptr);
			d_lookup_done(child);
			if (unlikely(res)) {
				dput(child);
				child = res;
				if (IS_ERR(child))
					goto end_instantiate;
			}
		}
	}
	inode = d_inode(child);
	ino = inode->i_ino;
	type = inode->i_mode >> 12;
	dput(child);
end_instantiate:
	return dir_emit(ctx, name, len, ino, type);
}

/*
 * dname_to_vma_addr - maps a dentry name into two unsigned longs
 * which represent vma start and end addresses.
 */
static int dname_to_vma_addr(struct dentry *dentry,
			     unsigned long *start, unsigned long *end)
{
	const char *str = dentry->d_name.name;
	unsigned long long sval, eval;
	unsigned int len;

	if (str[0] == '0' && str[1] != '-')
		return -EINVAL;
	len = _parse_integer(str, 16, &sval);
	if (len & KSTRTOX_OVERFLOW)
		return -EINVAL;
	if (sval != (unsigned long)sval)
		return -EINVAL;
	str += len;

	if (*str != '-')
		return -EINVAL;
	str++;

	if (str[0] == '0' && str[1])
		return -EINVAL;
	len = _parse_integer(str, 16, &eval);
	if (len & KSTRTOX_OVERFLOW)
		return -EINVAL;
	if (eval != (unsigned long)eval)
		return -EINVAL;
	str += len;

	if (*str != '\0')
		return -EINVAL;

	*start = sval;
	*end = eval;

	return 0;
}

static int map_files_d_revalidate(struct dentry *dentry, unsigned int flags)
{
	unsigned long vm_start, vm_end;
	bool exact_vma_exists = false;
	struct mm_struct *mm = NULL;
	struct task_struct *task;
	struct inode *inode;
	int status = 0;

	if (flags & LOOKUP_RCU)
		return -ECHILD;

	inode = d_inode(dentry);
	task = get_proc_task(inode);
	if (!task)
		goto out_notask;

	mm = mm_access(task, PTRACE_MODE_READ_FSCREDS);
	if (IS_ERR_OR_NULL(mm))
		goto out;

	if (!dname_to_vma_addr(dentry, &vm_start, &vm_end)) {
		status = down_read_killable(&mm->mmap_sem);
		if (!status) {
			exact_vma_exists = !!find_exact_vma(mm, vm_start,
							    vm_end);
			up_read(&mm->mmap_sem);
		}
	}

	mmput(mm);

	if (exact_vma_exists) {
		task_dump_owner(task, 0, &inode->i_uid, &inode->i_gid);

		security_task_to_inode(task, inode);
		status = 1;
	}

out:
	put_task_struct(task);

out_notask:
	return status;
}

static const struct dentry_operations tid_map_files_dentry_operations = {
	.d_revalidate	= map_files_d_revalidate,
	.d_delete	= pid_delete_dentry,
};

static int map_files_get_link(struct dentry *dentry, struct path *path)
{
	unsigned long vm_start, vm_end;
	struct vm_area_struct *vma;
	struct task_struct *task;
	struct mm_struct *mm;
	int rc;

	rc = -ENOENT;
	task = get_proc_task(d_inode(dentry));
	if (!task)
		goto out;

	mm = get_task_mm(task);
	put_task_struct(task);
	if (!mm)
		goto out;

	rc = dname_to_vma_addr(dentry, &vm_start, &vm_end);
	if (rc)
		goto out_mmput;

	rc = down_read_killable(&mm->mmap_sem);
	if (rc)
		goto out_mmput;

	rc = -ENOENT;
	vma = find_exact_vma(mm, vm_start, vm_end);
	if (vma && vma->vm_file) {
		*path = vma->vm_file->f_path;
		path_get(path);
		rc = 0;
	}
	up_read(&mm->mmap_sem);

out_mmput:
	mmput(mm);
out:
	return rc;
}

struct map_files_info {
	unsigned long	start;
	unsigned long	end;
	fmode_t		mode;
};

/*
 * Only allow CAP_SYS_ADMIN to follow the links, due to concerns about how the
 * symlinks may be used to bypass permissions on ancestor directories in the
 * path to the file in question.
 */
static const char *
proc_map_files_get_link(struct dentry *dentry,
			struct inode *inode,
		        struct delayed_call *done)
{
	if (!capable(CAP_SYS_ADMIN))
		return ERR_PTR(-EPERM);

	return proc_pid_get_link(dentry, inode, done);
}

/*
 * Identical to proc_pid_link_inode_operations except for get_link()
 */
static const struct inode_operations proc_map_files_link_inode_operations = {
	.readlink	= proc_pid_readlink,
	.get_link	= proc_map_files_get_link,
	.setattr	= proc_setattr,
};

static struct dentry *
proc_map_files_instantiate(struct dentry *dentry,
			   struct task_struct *task, const void *ptr)
{
	fmode_t mode = (fmode_t)(unsigned long)ptr;
	struct proc_inode *ei;
	struct inode *inode;

	inode = proc_pid_make_inode(dentry->d_sb, task, S_IFLNK |
				    ((mode & FMODE_READ ) ? S_IRUSR : 0) |
				    ((mode & FMODE_WRITE) ? S_IWUSR : 0));
	if (!inode)
		return ERR_PTR(-ENOENT);

	ei = PROC_I(inode);
	ei->op.proc_get_link = map_files_get_link;

	inode->i_op = &proc_map_files_link_inode_operations;
	inode->i_size = 64;

	d_set_d_op(dentry, &tid_map_files_dentry_operations);
	return d_splice_alias(inode, dentry);
}

static struct dentry *proc_map_files_lookup(struct inode *dir,
		struct dentry *dentry, unsigned int flags)
{
	unsigned long vm_start, vm_end;
	struct vm_area_struct *vma;
	struct task_struct *task;
	struct dentry *result;
	struct mm_struct *mm;

	result = ERR_PTR(-ENOENT);
	task = get_proc_task(dir);
	if (!task)
		goto out;

	result = ERR_PTR(-EACCES);
	if (!ptrace_may_access(task, PTRACE_MODE_READ_FSCREDS))
		goto out_put_task;

	result = ERR_PTR(-ENOENT);
	if (dname_to_vma_addr(dentry, &vm_start, &vm_end))
		goto out_put_task;

	mm = get_task_mm(task);
	if (!mm)
		goto out_put_task;

	result = ERR_PTR(-EINTR);
	if (down_read_killable(&mm->mmap_sem))
		goto out_put_mm;

	result = ERR_PTR(-ENOENT);
	vma = find_exact_vma(mm, vm_start, vm_end);
	if (!vma)
		goto out_no_vma;

	if (vma->vm_file)
		result = proc_map_files_instantiate(dentry, task,
				(void *)(unsigned long)vma->vm_file->f_mode);

out_no_vma:
	up_read(&mm->mmap_sem);
out_put_mm:
	mmput(mm);
out_put_task:
	put_task_struct(task);
out:
	return result;
}

static const struct inode_operations proc_map_files_inode_operations = {
	.lookup		= proc_map_files_lookup,
	.permission	= proc_fd_permission,
	.setattr	= proc_setattr,
};

static int
proc_map_files_readdir(struct file *file, struct dir_context *ctx)
{
	struct vm_area_struct *vma;
	struct task_struct *task;
	struct mm_struct *mm;
	unsigned long nr_files, pos, i;
	GENRADIX(struct map_files_info) fa;
	struct map_files_info *p;
	int ret;

	genradix_init(&fa);

	ret = -ENOENT;
	task = get_proc_task(file_inode(file));
	if (!task)
		goto out;

	ret = -EACCES;
	if (!ptrace_may_access(task, PTRACE_MODE_READ_FSCREDS))
		goto out_put_task;

	ret = 0;
	if (!dir_emit_dots(file, ctx))
		goto out_put_task;

	mm = get_task_mm(task);
	if (!mm)
		goto out_put_task;

	ret = down_read_killable(&mm->mmap_sem);
	if (ret) {
		mmput(mm);
		goto out_put_task;
	}

	nr_files = 0;

	/*
	 * We need two passes here:
	 *
	 *  1) Collect vmas of mapped files with mmap_sem taken
	 *  2) Release mmap_sem and instantiate entries
	 *
	 * otherwise we get lockdep complained, since filldir()
	 * routine might require mmap_sem taken in might_fault().
	 */

	for (vma = mm->mmap, pos = 2; vma; vma = vma->vm_next) {
		if (!vma->vm_file)
			continue;
		if (++pos <= ctx->pos)
			continue;

		p = genradix_ptr_alloc(&fa, nr_files++, GFP_KERNEL);
		if (!p) {
			ret = -ENOMEM;
			up_read(&mm->mmap_sem);
			mmput(mm);
			goto out_put_task;
		}

		p->start = vma->vm_start;
		p->end = vma->vm_end;
		p->mode = vma->vm_file->f_mode;
	}
	up_read(&mm->mmap_sem);
	mmput(mm);

	for (i = 0; i < nr_files; i++) {
		char buf[4 * sizeof(long) + 2];	/* max: %lx-%lx\0 */
		unsigned int len;

		p = genradix_ptr(&fa, i);
		len = snprintf(buf, sizeof(buf), "%lx-%lx", p->start, p->end);
		if (!proc_fill_cache(file, ctx,
				      buf, len,
				      proc_map_files_instantiate,
				      task,
				      (void *)(unsigned long)p->mode))
			break;
		ctx->pos++;
	}

out_put_task:
	put_task_struct(task);
out:
	genradix_free(&fa);
	return ret;
}

static const struct file_operations proc_map_files_operations = {
	.read		= generic_read_dir,
	.iterate_shared	= proc_map_files_readdir,
	.llseek		= generic_file_llseek,
};

#if defined(CONFIG_CHECKPOINT_RESTORE) && defined(CONFIG_POSIX_TIMERS)
struct timers_private {
	struct pid *pid;
	struct task_struct *task;
	struct sighand_struct *sighand;
	struct pid_namespace *ns;
	unsigned long flags;
};

static void *timers_start(struct seq_file *m, loff_t *pos)
{
	struct timers_private *tp = m->private;

	tp->task = get_pid_task(tp->pid, PIDTYPE_PID);
	if (!tp->task)
		return ERR_PTR(-ESRCH);

	tp->sighand = lock_task_sighand(tp->task, &tp->flags);
	if (!tp->sighand)
		return ERR_PTR(-ESRCH);

	return seq_list_start(&tp->task->signal->posix_timers, *pos);
}

static void *timers_next(struct seq_file *m, void *v, loff_t *pos)
{
	struct timers_private *tp = m->private;
	return seq_list_next(v, &tp->task->signal->posix_timers, pos);
}

static void timers_stop(struct seq_file *m, void *v)
{
	struct timers_private *tp = m->private;

	if (tp->sighand) {
		unlock_task_sighand(tp->task, &tp->flags);
		tp->sighand = NULL;
	}

	if (tp->task) {
		put_task_struct(tp->task);
		tp->task = NULL;
	}
}

static int show_timer(struct seq_file *m, void *v)
{
	struct k_itimer *timer;
	struct timers_private *tp = m->private;
	int notify;
	static const char * const nstr[] = {
		[SIGEV_SIGNAL] = "signal",
		[SIGEV_NONE] = "none",
		[SIGEV_THREAD] = "thread",
	};

	timer = list_entry((struct list_head *)v, struct k_itimer, list);
	notify = timer->it_sigev_notify;

	seq_printf(m, "ID: %d\n", timer->it_id);
	seq_printf(m, "signal: %d/%px\n",
		   timer->sigq->info.si_signo,
		   timer->sigq->info.si_value.sival_ptr);
	seq_printf(m, "notify: %s/%s.%d\n",
		   nstr[notify & ~SIGEV_THREAD_ID],
		   (notify & SIGEV_THREAD_ID) ? "tid" : "pid",
		   pid_nr_ns(timer->it_pid, tp->ns));
	seq_printf(m, "ClockID: %d\n", timer->it_clock);

	return 0;
}

static const struct seq_operations proc_timers_seq_ops = {
	.start	= timers_start,
	.next	= timers_next,
	.stop	= timers_stop,
	.show	= show_timer,
};

static int proc_timers_open(struct inode *inode, struct file *file)
{
	struct timers_private *tp;

	tp = __seq_open_private(file, &proc_timers_seq_ops,
			sizeof(struct timers_private));
	if (!tp)
		return -ENOMEM;

	tp->pid = proc_pid(inode);
	tp->ns = proc_pid_ns(inode);
	return 0;
}

static const struct file_operations proc_timers_operations = {
	.open		= proc_timers_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= seq_release_private,
};
#endif

static ssize_t timerslack_ns_write(struct file *file, const char __user *buf,
					size_t count, loff_t *offset)
{
	struct inode *inode = file_inode(file);
	struct task_struct *p;
	u64 slack_ns;
	int err;

	err = kstrtoull_from_user(buf, count, 10, &slack_ns);
	if (err < 0)
		return err;

	p = get_proc_task(inode);
	if (!p)
		return -ESRCH;

	if (p != current) {
		rcu_read_lock();
		if (!ns_capable(__task_cred(p)->user_ns, CAP_SYS_NICE)) {
			rcu_read_unlock();
			count = -EPERM;
			goto out;
		}
		rcu_read_unlock();

		err = security_task_setscheduler(p);
		if (err) {
			count = err;
			goto out;
		}
	}

	task_lock(p);
	if (slack_ns == 0)
		p->timer_slack_ns = p->default_timer_slack_ns;
	else
		p->timer_slack_ns = slack_ns;
	task_unlock(p);

out:
	put_task_struct(p);

	return count;
}

static int timerslack_ns_show(struct seq_file *m, void *v)
{
	struct inode *inode = m->private;
	struct task_struct *p;
	int err = 0;

	p = get_proc_task(inode);
	if (!p)
		return -ESRCH;

	if (p != current) {
		rcu_read_lock();
		if (!ns_capable(__task_cred(p)->user_ns, CAP_SYS_NICE)) {
			rcu_read_unlock();
			err = -EPERM;
			goto out;
		}
		rcu_read_unlock();

		err = security_task_getscheduler(p);
		if (err)
			goto out;
	}

	task_lock(p);
	seq_printf(m, "%llu\n", p->timer_slack_ns);
	task_unlock(p);

out:
	put_task_struct(p);

	return err;
}

static int timerslack_ns_open(struct inode *inode, struct file *filp)
{
	return single_open(filp, timerslack_ns_show, inode);
}

static const struct file_operations proc_pid_set_timerslack_ns_operations = {
	.open		= timerslack_ns_open,
	.read		= seq_read,
	.write		= timerslack_ns_write,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static struct dentry *proc_pident_instantiate(struct dentry *dentry,
	struct task_struct *task, const void *ptr)
{
	const struct pid_entry *p = ptr;
	struct inode *inode;
	struct proc_inode *ei;

	inode = proc_pid_make_inode(dentry->d_sb, task, p->mode);
	if (!inode)
		return ERR_PTR(-ENOENT);

	ei = PROC_I(inode);
	if (S_ISDIR(inode->i_mode))
		set_nlink(inode, 2);	/* Use getattr to fix if necessary */
	if (p->iop)
		inode->i_op = p->iop;
	if (p->fop)
		inode->i_fop = p->fop;
	ei->op = p->op;
	pid_update_inode(task, inode);
	d_set_d_op(dentry, &pid_dentry_operations);
	return d_splice_alias(inode, dentry);
}

static struct dentry *proc_pident_lookup(struct inode *dir, 
					 struct dentry *dentry,
					 const struct pid_entry *p,
					 const struct pid_entry *end)
{
	struct task_struct *task = get_proc_task(dir);
	struct dentry *res = ERR_PTR(-ENOENT);

	if (!task)
		goto out_no_task;

	/*
	 * Yes, it does not scale. And it should not. Don't add
	 * new entries into /proc/<tgid>/ without very good reasons.
	 */
	for (; p < end; p++) {
		if (p->len != dentry->d_name.len)
			continue;
		if (!memcmp(dentry->d_name.name, p->name, p->len)) {
			res = proc_pident_instantiate(dentry, task, p);
			break;
		}
	}
	put_task_struct(task);
out_no_task:
	return res;
}

static int proc_pident_readdir(struct file *file, struct dir_context *ctx,
		const struct pid_entry *ents, unsigned int nents)
{
	struct task_struct *task = get_proc_task(file_inode(file));
	const struct pid_entry *p;

	if (!task)
		return -ENOENT;

	if (!dir_emit_dots(file, ctx))
		goto out;

	if (ctx->pos >= nents + 2)
		goto out;

	for (p = ents + (ctx->pos - 2); p < ents + nents; p++) {
		if (!proc_fill_cache(file, ctx, p->name, p->len,
				proc_pident_instantiate, task, p))
			break;
		ctx->pos++;
	}
out:
	put_task_struct(task);
	return 0;
}

#ifdef CONFIG_SECURITY
static ssize_t proc_pid_attr_read(struct file * file, char __user * buf,
				  size_t count, loff_t *ppos)
{
	struct inode * inode = file_inode(file);
	char *p = NULL;
	ssize_t length;
	struct task_struct *task = get_proc_task(inode);

	if (!task)
		return -ESRCH;

	length = security_getprocattr(task, PROC_I(inode)->op.lsm,
				      (char*)file->f_path.dentry->d_name.name,
				      &p);
	put_task_struct(task);
	if (length > 0)
		length = simple_read_from_buffer(buf, count, ppos, p, length);
	kfree(p);
	return length;
}

static ssize_t proc_pid_attr_write(struct file * file, const char __user * buf,
				   size_t count, loff_t *ppos)
{
	struct inode * inode = file_inode(file);
	struct task_struct *task;
	void *page;
	int rv;

	rcu_read_lock();
	task = pid_task(proc_pid(inode), PIDTYPE_PID);
	if (!task) {
		rcu_read_unlock();
		return -ESRCH;
	}
	/* A task may only write its own attributes. */
	if (current != task) {
		rcu_read_unlock();
		return -EACCES;
	}
	/* Prevent changes to overridden credentials. */
	if (current_cred() != current_real_cred()) {
		rcu_read_unlock();
		return -EBUSY;
	}
	rcu_read_unlock();

	if (count > PAGE_SIZE)
		count = PAGE_SIZE;

	/* No partial writes. */
	if (*ppos != 0)
		return -EINVAL;

	page = memdup_user(buf, count);
	if (IS_ERR(page)) {
		rv = PTR_ERR(page);
		goto out;
	}

	/* Guard against adverse ptrace interaction */
	rv = mutex_lock_interruptible(&current->signal->cred_guard_mutex);
	if (rv < 0)
		goto out_free;

	rv = security_setprocattr(PROC_I(inode)->op.lsm,
				  file->f_path.dentry->d_name.name, page,
				  count);
	mutex_unlock(&current->signal->cred_guard_mutex);
out_free:
	kfree(page);
out:
	return rv;
}

static const struct file_operations proc_pid_attr_operations = {
	.read		= proc_pid_attr_read,
	.write		= proc_pid_attr_write,
	.llseek		= generic_file_llseek,
};

#define LSM_DIR_OPS(LSM) \
static int proc_##LSM##_attr_dir_iterate(struct file *filp, \
			     struct dir_context *ctx) \
{ \
	return proc_pident_readdir(filp, ctx, \
				   LSM##_attr_dir_stuff, \
				   ARRAY_SIZE(LSM##_attr_dir_stuff)); \
} \
\
static const struct file_operations proc_##LSM##_attr_dir_ops = { \
	.read		= generic_read_dir, \
	.iterate	= proc_##LSM##_attr_dir_iterate, \
	.llseek		= default_llseek, \
}; \
\
static struct dentry *proc_##LSM##_attr_dir_lookup(struct inode *dir, \
				struct dentry *dentry, unsigned int flags) \
{ \
	return proc_pident_lookup(dir, dentry, \
				  LSM##_attr_dir_stuff, \
				  LSM##_attr_dir_stuff + ARRAY_SIZE(LSM##_attr_dir_stuff)); \
} \
\
static const struct inode_operations proc_##LSM##_attr_dir_inode_ops = { \
	.lookup		= proc_##LSM##_attr_dir_lookup, \
	.getattr	= pid_getattr, \
	.setattr	= proc_setattr, \
}

#ifdef CONFIG_SECURITY_SMACK
static const struct pid_entry smack_attr_dir_stuff[] = {
	ATTR("smack", "current",	0666),
};
LSM_DIR_OPS(smack);
#endif

static const struct pid_entry attr_dir_stuff[] = {
	ATTR(NULL, "current",		0666),
	ATTR(NULL, "prev",		0444),
	ATTR(NULL, "exec",		0666),
	ATTR(NULL, "fscreate",		0666),
	ATTR(NULL, "keycreate",		0666),
	ATTR(NULL, "sockcreate",	0666),
#ifdef CONFIG_SECURITY_SMACK
	DIR("smack",			0555,
	    proc_smack_attr_dir_inode_ops, proc_smack_attr_dir_ops),
#endif
};

static int proc_attr_dir_readdir(struct file *file, struct dir_context *ctx)
{
	return proc_pident_readdir(file, ctx, 
				   attr_dir_stuff, ARRAY_SIZE(attr_dir_stuff));
}

static const struct file_operations proc_attr_dir_operations = {
	.read		= generic_read_dir,
	.iterate_shared	= proc_attr_dir_readdir,
	.llseek		= generic_file_llseek,
};

static struct dentry *proc_attr_dir_lookup(struct inode *dir,
				struct dentry *dentry, unsigned int flags)
{
	return proc_pident_lookup(dir, dentry,
				  attr_dir_stuff,
				  attr_dir_stuff + ARRAY_SIZE(attr_dir_stuff));
}

static const struct inode_operations proc_attr_dir_inode_operations = {
	.lookup		= proc_attr_dir_lookup,
	.getattr	= pid_getattr,
	.setattr	= proc_setattr,
};

#endif

#ifdef CONFIG_ELF_CORE
static ssize_t proc_coredump_filter_read(struct file *file, char __user *buf,
					 size_t count, loff_t *ppos)
{
	struct task_struct *task = get_proc_task(file_inode(file));
	struct mm_struct *mm;
	char buffer[PROC_NUMBUF];
	size_t len;
	int ret;

	if (!task)
		return -ESRCH;

	ret = 0;
	mm = get_task_mm(task);
	if (mm) {
		len = snprintf(buffer, sizeof(buffer), "%08lx\n",
			       ((mm->flags & MMF_DUMP_FILTER_MASK) >>
				MMF_DUMP_FILTER_SHIFT));
		mmput(mm);
		ret = simple_read_from_buffer(buf, count, ppos, buffer, len);
	}

	put_task_struct(task);

	return ret;
}

static ssize_t proc_coredump_filter_write(struct file *file,
					  const char __user *buf,
					  size_t count,
					  loff_t *ppos)
{
	struct task_struct *task;
	struct mm_struct *mm;
	unsigned int val;
	int ret;
	int i;
	unsigned long mask;

	ret = kstrtouint_from_user(buf, count, 0, &val);
	if (ret < 0)
		return ret;

	ret = -ESRCH;
	task = get_proc_task(file_inode(file));
	if (!task)
		goto out_no_task;

	mm = get_task_mm(task);
	if (!mm)
		goto out_no_mm;
	ret = 0;

	for (i = 0, mask = 1; i < MMF_DUMP_FILTER_BITS; i++, mask <<= 1) {
		if (val & mask)
			set_bit(i + MMF_DUMP_FILTER_SHIFT, &mm->flags);
		else
			clear_bit(i + MMF_DUMP_FILTER_SHIFT, &mm->flags);
	}

	mmput(mm);
 out_no_mm:
	put_task_struct(task);
 out_no_task:
	if (ret < 0)
		return ret;
	return count;
}

static const struct file_operations proc_coredump_filter_operations = {
	.read		= proc_coredump_filter_read,
	.write		= proc_coredump_filter_write,
	.llseek		= generic_file_llseek,
};
#endif

#ifdef CONFIG_TASK_IO_ACCOUNTING
static int do_io_accounting(struct task_struct *task, struct seq_file *m, int whole)
{
	struct task_io_accounting acct = task->ioac;
	unsigned long flags;
	int result;

	result = mutex_lock_killable(&task->signal->cred_guard_mutex);
	if (result)
		return result;

	if (!ptrace_may_access(task, PTRACE_MODE_READ_FSCREDS)) {
		result = -EACCES;
		goto out_unlock;
	}

	if (whole && lock_task_sighand(task, &flags)) {
		struct task_struct *t = task;

		task_io_accounting_add(&acct, &task->signal->ioac);
		while_each_thread(task, t)
			task_io_accounting_add(&acct, &t->ioac);

		unlock_task_sighand(task, &flags);
	}
	seq_printf(m,
		   "rchar: %llu\n"
		   "wchar: %llu\n"
		   "syscr: %llu\n"
		   "syscw: %llu\n"
		   "read_bytes: %llu\n"
		   "write_bytes: %llu\n"
		   "cancelled_write_bytes: %llu\n",
		   (unsigned long long)acct.rchar,
		   (unsigned long long)acct.wchar,
		   (unsigned long long)acct.syscr,
		   (unsigned long long)acct.syscw,
		   (unsigned long long)acct.read_bytes,
		   (unsigned long long)acct.write_bytes,
		   (unsigned long long)acct.cancelled_write_bytes);
	result = 0;

out_unlock:
	mutex_unlock(&task->signal->cred_guard_mutex);
	return result;
}

static int proc_tid_io_accounting(struct seq_file *m, struct pid_namespace *ns,
				  struct pid *pid, struct task_struct *task)
{
	return do_io_accounting(task, m, 0);
}

static int proc_tgid_io_accounting(struct seq_file *m, struct pid_namespace *ns,
				   struct pid *pid, struct task_struct *task)
{
	return do_io_accounting(task, m, 1);
}
#endif /* CONFIG_TASK_IO_ACCOUNTING */

#ifdef CONFIG_USER_NS
static int proc_id_map_open(struct inode *inode, struct file *file,
	const struct seq_operations *seq_ops)
{
	struct user_namespace *ns = NULL;
	struct task_struct *task;
	struct seq_file *seq;
	int ret = -EINVAL;

	task = get_proc_task(inode);
	if (task) {
		rcu_read_lock();
		ns = get_user_ns(task_cred_xxx(task, user_ns));
		rcu_read_unlock();
		put_task_struct(task);
	}
	if (!ns)
		goto err;

	ret = seq_open(file, seq_ops);
	if (ret)
		goto err_put_ns;

	seq = file->private_data;
	seq->private = ns;

	return 0;
err_put_ns:
	put_user_ns(ns);
err:
	return ret;
}

static int proc_id_map_release(struct inode *inode, struct file *file)
{
	struct seq_file *seq = file->private_data;
	struct user_namespace *ns = seq->private;
	put_user_ns(ns);
	return seq_release(inode, file);
}

static int proc_uid_map_open(struct inode *inode, struct file *file)
{
	return proc_id_map_open(inode, file, &proc_uid_seq_operations);
}

static int proc_gid_map_open(struct inode *inode, struct file *file)
{
	return proc_id_map_open(inode, file, &proc_gid_seq_operations);
}

static int proc_projid_map_open(struct inode *inode, struct file *file)
{
	return proc_id_map_open(inode, file, &proc_projid_seq_operations);
}

static const struct file_operations proc_uid_map_operations = {
	.open		= proc_uid_map_open,
	.write		= proc_uid_map_write,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= proc_id_map_release,
};

static const struct file_operations proc_gid_map_operations = {
	.open		= proc_gid_map_open,
	.write		= proc_gid_map_write,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= proc_id_map_release,
};

static const struct file_operations proc_projid_map_operations = {
	.open		= proc_projid_map_open,
	.write		= proc_projid_map_write,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= proc_id_map_release,
};

static int proc_setgroups_open(struct inode *inode, struct file *file)
{
	struct user_namespace *ns = NULL;
	struct task_struct *task;
	int ret;

	ret = -ESRCH;
	task = get_proc_task(inode);
	if (task) {
		rcu_read_lock();
		ns = get_user_ns(task_cred_xxx(task, user_ns));
		rcu_read_unlock();
		put_task_struct(task);
	}
	if (!ns)
		goto err;

	if (file->f_mode & FMODE_WRITE) {
		ret = -EACCES;
		if (!ns_capable(ns, CAP_SYS_ADMIN))
			goto err_put_ns;
	}

	ret = single_open(file, &proc_setgroups_show, ns);
	if (ret)
		goto err_put_ns;

	return 0;
err_put_ns:
	put_user_ns(ns);
err:
	return ret;
}

static int proc_setgroups_release(struct inode *inode, struct file *file)
{
	struct seq_file *seq = file->private_data;
	struct user_namespace *ns = seq->private;
	int ret = single_release(inode, file);
	put_user_ns(ns);
	return ret;
}

static const struct file_operations proc_setgroups_operations = {
	.open		= proc_setgroups_open,
	.write		= proc_setgroups_write,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= proc_setgroups_release,
};
#endif /* CONFIG_USER_NS */

static int proc_pid_personality(struct seq_file *m, struct pid_namespace *ns,
				struct pid *pid, struct task_struct *task)
{
	int err = lock_trace(task);
	if (!err) {
		seq_printf(m, "%08x\n", task->personality);
		unlock_trace(task);
	}
	return err;
}

#ifdef CONFIG_LIVEPATCH
static int proc_pid_patch_state(struct seq_file *m, struct pid_namespace *ns,
				struct pid *pid, struct task_struct *task)
{
	seq_printf(m, "%d\n", task->patch_state);
	return 0;
}
#endif /* CONFIG_LIVEPATCH */

#ifdef CONFIG_STACKLEAK_METRICS
static int proc_stack_depth(struct seq_file *m, struct pid_namespace *ns,
				struct pid *pid, struct task_struct *task)
{
	unsigned long prev_depth = THREAD_SIZE -
				(task->prev_lowest_stack & (THREAD_SIZE - 1));
	unsigned long depth = THREAD_SIZE -
				(task->lowest_stack & (THREAD_SIZE - 1));

	seq_printf(m, "previous stack depth: %lu\nstack depth: %lu\n",
							prev_depth, depth);
	return 0;
}
#endif /* CONFIG_STACKLEAK_METRICS */

/*
 * Thread groups
 */
static const struct file_operations proc_task_operations;
static const struct inode_operations proc_task_inode_operations;

static const struct pid_entry tgid_base_stuff[] = {
	DIR("task",       S_IRUGO|S_IXUGO, proc_task_inode_operations, proc_task_operations),
	DIR("fd",         S_IRUSR|S_IXUSR, proc_fd_inode_operations, proc_fd_operations),
	DIR("map_files",  S_IRUSR|S_IXUSR, proc_map_files_inode_operations, proc_map_files_operations),
	DIR("fdinfo",     S_IRUSR|S_IXUSR, proc_fdinfo_inode_operations, proc_fdinfo_operations),
	DIR("ns",	  S_IRUSR|S_IXUGO, proc_ns_dir_inode_operations, proc_ns_dir_operations),
#ifdef CONFIG_NET
	DIR("net",        S_IRUGO|S_IXUGO, proc_net_inode_operations, proc_net_operations),
#endif
	REG("environ",    S_IRUSR, proc_environ_operations),
	REG("auxv",       S_IRUSR, proc_auxv_operations),
	ONE("status",     S_IRUGO, proc_pid_status),
	ONE("personality", S_IRUSR, proc_pid_personality),
	ONE("limits",	  S_IRUGO, proc_pid_limits),
#ifdef CONFIG_SCHED_DEBUG
	REG("sched",      S_IRUGO|S_IWUSR, proc_pid_sched_operations),
#endif
#ifdef CONFIG_SCHED_AUTOGROUP
	REG("autogroup",  S_IRUGO|S_IWUSR, proc_pid_sched_autogroup_operations),
#endif
#ifdef CONFIG_TIME_NS
	REG("timens_offsets",  S_IRUGO|S_IWUSR, proc_timens_offsets_operations),
#endif
	REG("comm",      S_IRUGO|S_IWUSR, proc_pid_set_comm_operations),
#ifdef CONFIG_HAVE_ARCH_TRACEHOOK
	ONE("syscall",    S_IRUSR, proc_pid_syscall),
#endif
	REG("cmdline",    S_IRUGO, proc_pid_cmdline_ops),
	ONE("stat",       S_IRUGO, proc_tgid_stat),
	ONE("statm",      S_IRUGO, proc_pid_statm),
	REG("maps",       S_IRUGO, proc_pid_maps_operations),
#ifdef CONFIG_NUMA
	REG("numa_maps",  S_IRUGO, proc_pid_numa_maps_operations),
#endif
	REG("mem",        S_IRUSR|S_IWUSR, proc_mem_operations),
	LNK("cwd",        proc_cwd_link),
	LNK("root",       proc_root_link),
	LNK("exe",        proc_exe_link),
	REG("mounts",     S_IRUGO, proc_mounts_operations),
	REG("mountinfo",  S_IRUGO, proc_mountinfo_operations),
	REG("mountstats", S_IRUSR, proc_mountstats_operations),
#ifdef CONFIG_PROC_PAGE_MONITOR
	REG("clear_refs", S_IWUSR, proc_clear_refs_operations),
	REG("smaps",      S_IRUGO, proc_pid_smaps_operations),
	REG("smaps_rollup", S_IRUGO, proc_pid_smaps_rollup_operations),
	REG("pagemap",    S_IRUSR, proc_pagemap_operations),
#endif
#ifdef CONFIG_SECURITY
	DIR("attr",       S_IRUGO|S_IXUGO, proc_attr_dir_inode_operations, proc_attr_dir_operations),
#endif
#ifdef CONFIG_KALLSYMS
	ONE("wchan",      S_IRUGO, proc_pid_wchan),
#endif
#ifdef CONFIG_STACKTRACE
	ONE("stack",      S_IRUSR, proc_pid_stack),
#endif
#ifdef CONFIG_SCHED_INFO
	ONE("schedstat",  S_IRUGO, proc_pid_schedstat),
#endif
#ifdef CONFIG_LATENCYTOP
	REG("latency",  S_IRUGO, proc_lstats_operations),
#endif
#ifdef CONFIG_PROC_PID_CPUSET
	ONE("cpuset",     S_IRUGO, proc_cpuset_show),
#endif
#ifdef CONFIG_CGROUPS
	ONE("cgroup",  S_IRUGO, proc_cgroup_show),
#endif
#ifdef CONFIG_PROC_CPU_RESCTRL
	ONE("cpu_resctrl_groups", S_IRUGO, proc_resctrl_show),
#endif
	ONE("oom_score",  S_IRUGO, proc_oom_score),
	REG("oom_adj",    S_IRUGO|S_IWUSR, proc_oom_adj_operations),
	REG("oom_score_adj", S_IRUGO|S_IWUSR, proc_oom_score_adj_operations),
#ifdef CONFIG_AUDIT
	REG("loginuid",   S_IWUSR|S_IRUGO, proc_loginuid_operations),
	REG("sessionid",  S_IRUGO, proc_sessionid_operations),
#endif
#ifdef CONFIG_FAULT_INJECTION
	REG("make-it-fail", S_IRUGO|S_IWUSR, proc_fault_inject_operations),
	REG("fail-nth", 0644, proc_fail_nth_operations),
#endif
#ifdef CONFIG_ELF_CORE
	REG("coredump_filter", S_IRUGO|S_IWUSR, proc_coredump_filter_operations),
#endif
#ifdef CONFIG_TASK_IO_ACCOUNTING
	ONE("io",	S_IRUSR, proc_tgid_io_accounting),
#endif
#ifdef CONFIG_USER_NS
	REG("uid_map",    S_IRUGO|S_IWUSR, proc_uid_map_operations),
	REG("gid_map",    S_IRUGO|S_IWUSR, proc_gid_map_operations),
	REG("projid_map", S_IRUGO|S_IWUSR, proc_projid_map_operations),
	REG("setgroups",  S_IRUGO|S_IWUSR, proc_setgroups_operations),
#endif
#if defined(CONFIG_CHECKPOINT_RESTORE) && defined(CONFIG_POSIX_TIMERS)
	REG("timers",	  S_IRUGO, proc_timers_operations),
#endif
	REG("timerslack_ns", S_IRUGO|S_IWUGO, proc_pid_set_timerslack_ns_operations),
#ifdef CONFIG_LIVEPATCH
	ONE("patch_state",  S_IRUSR, proc_pid_patch_state),
#endif
#ifdef CONFIG_STACKLEAK_METRICS
	ONE("stack_depth", S_IRUGO, proc_stack_depth),
#endif
#ifdef CONFIG_PROC_PID_ARCH_STATUS
	ONE("arch_status", S_IRUGO, proc_pid_arch_status),
#endif
};

static int proc_tgid_base_readdir(struct file *file, struct dir_context *ctx)
{
	return proc_pident_readdir(file, ctx,
				   tgid_base_stuff, ARRAY_SIZE(tgid_base_stuff));
}

static const struct file_operations proc_tgid_base_operations = {
	.read		= generic_read_dir,
	.iterate_shared	= proc_tgid_base_readdir,
	.llseek		= generic_file_llseek,
};

struct pid *tgid_pidfd_to_pid(const struct file *file)
{
	if (file->f_op != &proc_tgid_base_operations)
		return ERR_PTR(-EBADF);

	return proc_pid(file_inode(file));
}

static struct dentry *proc_tgid_base_lookup(struct inode *dir, struct dentry *dentry, unsigned int flags)
{
	return proc_pident_lookup(dir, dentry,
				  tgid_base_stuff,
				  tgid_base_stuff + ARRAY_SIZE(tgid_base_stuff));
}

static const struct inode_operations proc_tgid_base_inode_operations = {
	.lookup		= proc_tgid_base_lookup,
	.getattr	= pid_getattr,
	.setattr	= proc_setattr,
	.permission	= proc_pid_permission,
};

static void proc_flush_task_mnt(struct vfsmount *mnt, pid_t pid, pid_t tgid)
{
	struct dentry *dentry, *leader, *dir;
	char buf[10 + 1];
	struct qstr name;

	name.name = buf;
	name.len = snprintf(buf, sizeof(buf), "%u", pid);
	/* no ->d_hash() rejects on procfs */
	dentry = d_hash_and_lookup(mnt->mnt_root, &name);
	if (dentry) {
		d_invalidate(dentry);
		dput(dentry);
	}

	if (pid == tgid)
		return;

	name.name = buf;
	name.len = snprintf(buf, sizeof(buf), "%u", tgid);
	leader = d_hash_and_lookup(mnt->mnt_root, &name);
	if (!leader)
		goto out;

	name.name = "task";
	name.len = strlen(name.name);
	dir = d_hash_and_lookup(leader, &name);
	if (!dir)
		goto out_put_leader;

	name.name = buf;
	name.len = snprintf(buf, sizeof(buf), "%u", pid);
	dentry = d_hash_and_lookup(dir, &name);
	if (dentry) {
		d_invalidate(dentry);
		dput(dentry);
	}

	dput(dir);
out_put_leader:
	dput(leader);
out:
	return;
}

/**
 * proc_flush_task -  Remove dcache entries for @task from the /proc dcache.
 * @task: task that should be flushed.
 *
 * When flushing dentries from proc, one needs to flush them from global
 * proc (proc_mnt) and from all the namespaces' procs this task was seen
 * in. This call is supposed to do all of this job.
 *
 * Looks in the dcache for
 * /proc/@pid
 * /proc/@tgid/task/@pid
 * if either directory is present flushes it and all of it'ts children
 * from the dcache.
 *
 * It is safe and reasonable to cache /proc entries for a task until
 * that task exits.  After that they just clog up the dcache with
 * useless entries, possibly causing useful dcache entries to be
 * flushed instead.  This routine is proved to flush those useless
 * dcache entries at process exit time.
 *
 * NOTE: This routine is just an optimization so it does not guarantee
 *       that no dcache entries will exist at process exit time it
 *       just makes it very unlikely that any will persist.
 */

void proc_flush_task(struct task_struct *task)
{
	int i;
	struct pid *pid, *tgid;
	struct upid *upid;

	pid = task_pid(task);
	tgid = task_tgid(task);

	for (i = 0; i <= pid->level; i++) {
		upid = &pid->numbers[i];
		proc_flush_task_mnt(upid->ns->proc_mnt, upid->nr,
					tgid->numbers[i].nr);
	}
}

static struct dentry *proc_pid_instantiate(struct dentry * dentry,
				   struct task_struct *task, const void *ptr)
{
	struct inode *inode;

	inode = proc_pid_make_inode(dentry->d_sb, task, S_IFDIR | S_IRUGO | S_IXUGO);
	if (!inode)
		return ERR_PTR(-ENOENT);

	inode->i_op = &proc_tgid_base_inode_operations;
	inode->i_fop = &proc_tgid_base_operations;
	inode->i_flags|=S_IMMUTABLE;

	set_nlink(inode, nlink_tgid);
	pid_update_inode(task, inode);

	d_set_d_op(dentry, &pid_dentry_operations);
	return d_splice_alias(inode, dentry);
}

struct dentry *proc_pid_lookup(struct dentry *dentry, unsigned int flags)
{
	struct task_struct *task;
	unsigned tgid;
	struct pid_namespace *ns;
	struct dentry *result = ERR_PTR(-ENOENT);

	tgid = name_to_int(&dentry->d_name);
	if (tgid == ~0U)
		goto out;

	ns = dentry->d_sb->s_fs_info;
	rcu_read_lock();
	task = find_task_by_pid_ns(tgid, ns);
	if (task)
		get_task_struct(task);
	rcu_read_unlock();
	if (!task)
		goto out;

	result = proc_pid_instantiate(dentry, task, NULL);
	put_task_struct(task);
out:
	return result;
}

/*
 * Find the first task with tgid >= tgid
 *
 */
struct tgid_iter {
	unsigned int tgid;
	struct task_struct *task;
};
static struct tgid_iter next_tgid(struct pid_namespace *ns, struct tgid_iter iter)
{
	struct pid *pid;

	if (iter.task)
		put_task_struct(iter.task);
	rcu_read_lock();
retry:
	iter.task = NULL;
	pid = find_ge_pid(iter.tgid, ns);
	if (pid) {
		iter.tgid = pid_nr_ns(pid, ns);
		iter.task = pid_task(pid, PIDTYPE_PID);
		/* What we to know is if the pid we have find is the
		 * pid of a thread_group_leader.  Testing for task
		 * being a thread_group_leader is the obvious thing
		 * todo but there is a window when it fails, due to
		 * the pid transfer logic in de_thread.
		 *
		 * So we perform the straight forward test of seeing
		 * if the pid we have found is the pid of a thread
		 * group leader, and don't worry if the task we have
		 * found doesn't happen to be a thread group leader.
		 * As we don't care in the case of readdir.
		 */
		if (!iter.task || !has_group_leader_pid(iter.task)) {
			iter.tgid += 1;
			goto retry;
		}
		get_task_struct(iter.task);
	}
	rcu_read_unlock();
	return iter;
}

#define TGID_OFFSET (FIRST_PROCESS_ENTRY + 2)

/* for the /proc/ directory itself, after non-process stuff has been done */
int proc_pid_readdir(struct file *file, struct dir_context *ctx)
{
	struct tgid_iter iter;
	struct pid_namespace *ns = proc_pid_ns(file_inode(file));
	loff_t pos = ctx->pos;

	if (pos >= PID_MAX_LIMIT + TGID_OFFSET)
		return 0;

	if (pos == TGID_OFFSET - 2) {
		struct inode *inode = d_inode(ns->proc_self);
		if (!dir_emit(ctx, "self", 4, inode->i_ino, DT_LNK))
			return 0;
		ctx->pos = pos = pos + 1;
	}
	if (pos == TGID_OFFSET - 1) {
		struct inode *inode = d_inode(ns->proc_thread_self);
		if (!dir_emit(ctx, "thread-self", 11, inode->i_ino, DT_LNK))
			return 0;
		ctx->pos = pos = pos + 1;
	}
	iter.tgid = pos - TGID_OFFSET;
	iter.task = NULL;
	for (iter = next_tgid(ns, iter);
	     iter.task;
	     iter.tgid += 1, iter = next_tgid(ns, iter)) {
		char name[10 + 1];
		unsigned int len;

		cond_resched();
		if (!has_pid_permissions(ns, iter.task, HIDEPID_INVISIBLE))
			continue;

		len = snprintf(name, sizeof(name), "%u", iter.tgid);
		ctx->pos = iter.tgid + TGID_OFFSET;
		if (!proc_fill_cache(file, ctx, name, len,
				     proc_pid_instantiate, iter.task, NULL)) {
			put_task_struct(iter.task);
			return 0;
		}
	}
	ctx->pos = PID_MAX_LIMIT + TGID_OFFSET;
	return 0;
}

/*
 * proc_tid_comm_permission is a special permission function exclusively
 * used for the node /proc/<pid>/task/<tid>/comm.
 * It bypasses generic permission checks in the case where a task of the same
 * task group attempts to access the node.
 * The rationale behind this is that glibc and bionic access this node for
 * cross thread naming (pthread_set/getname_np(!self)). However, if
 * PR_SET_DUMPABLE gets set to 0 this node among others becomes uid=0 gid=0,
 * which locks out the cross thread naming implementation.
 * This function makes sure that the node is always accessible for members of
 * same thread group.
 */
static int proc_tid_comm_permission(struct inode *inode, int mask)
{
	bool is_same_tgroup;
	struct task_struct *task;

	task = get_proc_task(inode);
	if (!task)
		return -ESRCH;
	is_same_tgroup = same_thread_group(current, task);
	put_task_struct(task);

	if (likely(is_same_tgroup && !(mask & MAY_EXEC))) {
		/* This file (/proc/<pid>/task/<tid>/comm) can always be
		 * read or written by the members of the corresponding
		 * thread group.
		 */
		return 0;
	}

	return generic_permission(inode, mask);
}

static const struct inode_operations proc_tid_comm_inode_operations = {
		.permission = proc_tid_comm_permission,
};

/*
 * Tasks
 */
static const struct pid_entry tid_base_stuff[] = {
	DIR("fd",        S_IRUSR|S_IXUSR, proc_fd_inode_operations, proc_fd_operations),
	DIR("fdinfo",    S_IRUSR|S_IXUSR, proc_fdinfo_inode_operations, proc_fdinfo_operations),
	DIR("ns",	 S_IRUSR|S_IXUGO, proc_ns_dir_inode_operations, proc_ns_dir_operations),
#ifdef CONFIG_NET
	DIR("net",        S_IRUGO|S_IXUGO, proc_net_inode_operations, proc_net_operations),
#endif
	REG("environ",   S_IRUSR, proc_environ_operations),
	REG("auxv",      S_IRUSR, proc_auxv_operations),
	ONE("status",    S_IRUGO, proc_pid_status),
	ONE("personality", S_IRUSR, proc_pid_personality),
	ONE("limits",	 S_IRUGO, proc_pid_limits),
#ifdef CONFIG_SCHED_DEBUG
	REG("sched",     S_IRUGO|S_IWUSR, proc_pid_sched_operations),
#endif
	NOD("comm",      S_IFREG|S_IRUGO|S_IWUSR,
			 &proc_tid_comm_inode_operations,
			 &proc_pid_set_comm_operations, {}),
#ifdef CONFIG_HAVE_ARCH_TRACEHOOK
	ONE("syscall",   S_IRUSR, proc_pid_syscall),
#endif
	REG("cmdline",   S_IRUGO, proc_pid_cmdline_ops),
	ONE("stat",      S_IRUGO, proc_tid_stat),
	ONE("statm",     S_IRUGO, proc_pid_statm),
	REG("maps",      S_IRUGO, proc_pid_maps_operations),
#ifdef CONFIG_PROC_CHILDREN
	REG("children",  S_IRUGO, proc_tid_children_operations),
#endif
#ifdef CONFIG_NUMA
	REG("numa_maps", S_IRUGO, proc_pid_numa_maps_operations),
#endif
	REG("mem",       S_IRUSR|S_IWUSR, proc_mem_operations),
	LNK("cwd",       proc_cwd_link),
	LNK("root",      proc_root_link),
	LNK("exe",       proc_exe_link),
	REG("mounts",    S_IRUGO, proc_mounts_operations),
	REG("mountinfo",  S_IRUGO, proc_mountinfo_operations),
#ifdef CONFIG_PROC_PAGE_MONITOR
	REG("clear_refs", S_IWUSR, proc_clear_refs_operations),
	REG("smaps",     S_IRUGO, proc_pid_smaps_operations),
	REG("smaps_rollup", S_IRUGO, proc_pid_smaps_rollup_operations),
	REG("pagemap",    S_IRUSR, proc_pagemap_operations),
#endif
#ifdef CONFIG_SECURITY
	DIR("attr",      S_IRUGO|S_IXUGO, proc_attr_dir_inode_operations, proc_attr_dir_operations),
#endif
#ifdef CONFIG_KALLSYMS
	ONE("wchan",     S_IRUGO, proc_pid_wchan),
#endif
#ifdef CONFIG_STACKTRACE
	ONE("stack",      S_IRUSR, proc_pid_stack),
#endif
#ifdef CONFIG_SCHED_INFO
	ONE("schedstat", S_IRUGO, proc_pid_schedstat),
#endif
#ifdef CONFIG_LATENCYTOP
	REG("latency",  S_IRUGO, proc_lstats_operations),
#endif
#ifdef CONFIG_PROC_PID_CPUSET
	ONE("cpuset",    S_IRUGO, proc_cpuset_show),
#endif
#ifdef CONFIG_CGROUPS
	ONE("cgroup",  S_IRUGO, proc_cgroup_show),
#endif
#ifdef CONFIG_PROC_CPU_RESCTRL
	ONE("cpu_resctrl_groups", S_IRUGO, proc_resctrl_show),
#endif
	ONE("oom_score", S_IRUGO, proc_oom_score),
	REG("oom_adj",   S_IRUGO|S_IWUSR, proc_oom_adj_operations),
	REG("oom_score_adj", S_IRUGO|S_IWUSR, proc_oom_score_adj_operations),
#ifdef CONFIG_AUDIT
	REG("loginuid",  S_IWUSR|S_IRUGO, proc_loginuid_operations),
	REG("sessionid",  S_IRUGO, proc_sessionid_operations),
#endif
#ifdef CONFIG_FAULT_INJECTION
	REG("make-it-fail", S_IRUGO|S_IWUSR, proc_fault_inject_operations),
	REG("fail-nth", 0644, proc_fail_nth_operations),
#endif
#ifdef CONFIG_TASK_IO_ACCOUNTING
	ONE("io",	S_IRUSR, proc_tid_io_accounting),
#endif
#ifdef CONFIG_USER_NS
	REG("uid_map",    S_IRUGO|S_IWUSR, proc_uid_map_operations),
	REG("gid_map",    S_IRUGO|S_IWUSR, proc_gid_map_operations),
	REG("projid_map", S_IRUGO|S_IWUSR, proc_projid_map_operations),
	REG("setgroups",  S_IRUGO|S_IWUSR, proc_setgroups_operations),
#endif
#ifdef CONFIG_LIVEPATCH
	ONE("patch_state",  S_IRUSR, proc_pid_patch_state),
#endif
#ifdef CONFIG_PROC_PID_ARCH_STATUS
	ONE("arch_status", S_IRUGO, proc_pid_arch_status),
#endif
};

static int proc_tid_base_readdir(struct file *file, struct dir_context *ctx)
{
	return proc_pident_readdir(file, ctx,
				   tid_base_stuff, ARRAY_SIZE(tid_base_stuff));
}

static struct dentry *proc_tid_base_lookup(struct inode *dir, struct dentry *dentry, unsigned int flags)
{
	return proc_pident_lookup(dir, dentry,
				  tid_base_stuff,
				  tid_base_stuff + ARRAY_SIZE(tid_base_stuff));
}

static const struct file_operations proc_tid_base_operations = {
	.read		= generic_read_dir,
	.iterate_shared	= proc_tid_base_readdir,
	.llseek		= generic_file_llseek,
};

static const struct inode_operations proc_tid_base_inode_operations = {
	.lookup		= proc_tid_base_lookup,
	.getattr	= pid_getattr,
	.setattr	= proc_setattr,
};

static struct dentry *proc_task_instantiate(struct dentry *dentry,
	struct task_struct *task, const void *ptr)
{
	struct inode *inode;
	inode = proc_pid_make_inode(dentry->d_sb, task, S_IFDIR | S_IRUGO | S_IXUGO);
	if (!inode)
		return ERR_PTR(-ENOENT);

	inode->i_op = &proc_tid_base_inode_operations;
	inode->i_fop = &proc_tid_base_operations;
	inode->i_flags |= S_IMMUTABLE;

	set_nlink(inode, nlink_tid);
	pid_update_inode(task, inode);

	d_set_d_op(dentry, &pid_dentry_operations);
	return d_splice_alias(inode, dentry);
}

static struct dentry *proc_task_lookup(struct inode *dir, struct dentry * dentry, unsigned int flags)
{
	struct task_struct *task;
	struct task_struct *leader = get_proc_task(dir);
	unsigned tid;
	struct pid_namespace *ns;
	struct dentry *result = ERR_PTR(-ENOENT);

	if (!leader)
		goto out_no_task;

	tid = name_to_int(&dentry->d_name);
	if (tid == ~0U)
		goto out;

	ns = dentry->d_sb->s_fs_info;
	rcu_read_lock();
	task = find_task_by_pid_ns(tid, ns);
	if (task)
		get_task_struct(task);
	rcu_read_unlock();
	if (!task)
		goto out;
	if (!same_thread_group(leader, task))
		goto out_drop_task;

	result = proc_task_instantiate(dentry, task, NULL);
out_drop_task:
	put_task_struct(task);
out:
	put_task_struct(leader);
out_no_task:
	return result;
}

/*
 * Find the first tid of a thread group to return to user space.
 *
 * Usually this is just the thread group leader, but if the users
 * buffer was too small or there was a seek into the middle of the
 * directory we have more work todo.
 *
 * In the case of a short read we start with find_task_by_pid.
 *
 * In the case of a seek we start with the leader and walk nr
 * threads past it.
 */
static struct task_struct *first_tid(struct pid *pid, int tid, loff_t f_pos,
					struct pid_namespace *ns)
{
	struct task_struct *pos, *task;
	unsigned long nr = f_pos;

	if (nr != f_pos)	/* 32bit overflow? */
		return NULL;

	rcu_read_lock();
	task = pid_task(pid, PIDTYPE_PID);
	if (!task)
		goto fail;

	/* Attempt to start with the tid of a thread */
	if (tid && nr) {
		pos = find_task_by_pid_ns(tid, ns);
		if (pos && same_thread_group(pos, task))
			goto found;
	}

	/* If nr exceeds the number of threads there is nothing todo */
	if (nr >= get_nr_threads(task))
		goto fail;

	/* If we haven't found our starting place yet start
	 * with the leader and walk nr threads forward.
	 */
	pos = task = task->group_leader;
	do {
		if (!nr--)
			goto found;
	} while_each_thread(task, pos);
fail:
	pos = NULL;
	goto out;
found:
	get_task_struct(pos);
out:
	rcu_read_unlock();
	return pos;
}

/*
 * Find the next thread in the thread list.
 * Return NULL if there is an error or no next thread.
 *
 * The reference to the input task_struct is released.
 */
static struct task_struct *next_tid(struct task_struct *start)
{
	struct task_struct *pos = NULL;
	rcu_read_lock();
	if (pid_alive(start)) {
		pos = next_thread(start);
		if (thread_group_leader(pos))
			pos = NULL;
		else
			get_task_struct(pos);
	}
	rcu_read_unlock();
	put_task_struct(start);
	return pos;
}

/* for the /proc/TGID/task/ directories */
static int proc_task_readdir(struct file *file, struct dir_context *ctx)
{
	struct inode *inode = file_inode(file);
	struct task_struct *task;
	struct pid_namespace *ns;
	int tid;

	if (proc_inode_is_dead(inode))
		return -ENOENT;

	if (!dir_emit_dots(file, ctx))
		return 0;

	/* f_version caches the tgid value that the last readdir call couldn't
	 * return. lseek aka telldir automagically resets f_version to 0.
	 */
	ns = proc_pid_ns(inode);
	tid = (int)file->f_version;
	file->f_version = 0;
	for (task = first_tid(proc_pid(inode), tid, ctx->pos - 2, ns);
	     task;
	     task = next_tid(task), ctx->pos++) {
		char name[10 + 1];
		unsigned int len;
		tid = task_pid_nr_ns(task, ns);
		len = snprintf(name, sizeof(name), "%u", tid);
		if (!proc_fill_cache(file, ctx, name, len,
				proc_task_instantiate, task, NULL)) {
			/* returning this tgid failed, save it as the first
			 * pid for the next readir call */
			file->f_version = (u64)tid;
			put_task_struct(task);
			break;
		}
	}

	return 0;
}

static int proc_task_getattr(const struct path *path, struct kstat *stat,
			     u32 request_mask, unsigned int query_flags)
{
	struct inode *inode = d_inode(path->dentry);
	struct task_struct *p = get_proc_task(inode);
	generic_fillattr(inode, stat);

	if (p) {
		stat->nlink += get_nr_threads(p);
		put_task_struct(p);
	}

	return 0;
}

static const struct inode_operations proc_task_inode_operations = {
	.lookup		= proc_task_lookup,
	.getattr	= proc_task_getattr,
	.setattr	= proc_setattr,
	.permission	= proc_pid_permission,
};

static const struct file_operations proc_task_operations = {
	.read		= generic_read_dir,
	.iterate_shared	= proc_task_readdir,
	.llseek		= generic_file_llseek,
};

void __init set_proc_pid_nlink(void)
{
	nlink_tid = pid_entry_nlink(tid_base_stuff, ARRAY_SIZE(tid_base_stuff));
	nlink_tgid = pid_entry_nlink(tgid_base_stuff, ARRAY_SIZE(tgid_base_stuff));
}