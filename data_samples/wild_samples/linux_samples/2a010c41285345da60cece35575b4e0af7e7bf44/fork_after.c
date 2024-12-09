{
	struct file *exe_file;

	exe_file = get_mm_exe_file(oldmm);
	RCU_INIT_POINTER(mm->exe_file, exe_file);
}

#ifdef CONFIG_MMU
static __latent_entropy int dup_mmap(struct mm_struct *mm,
					struct mm_struct *oldmm)
{
	struct vm_area_struct *mpnt, *tmp;
	int retval;
	unsigned long charge = 0;
	LIST_HEAD(uf);
	VMA_ITERATOR(vmi, mm, 0);

	uprobe_start_dup_mmap();
	if (mmap_write_lock_killable(oldmm)) {
		retval = -EINTR;
		goto fail_uprobe_end;
	}
{
	struct file *old_exe_file;

	/*
	 * It is safe to dereference the exe_file without RCU as
	 * this function is only called if nobody else can access
	 * this mm -- see comment above for justification.
	 */
	old_exe_file = rcu_dereference_raw(mm->exe_file);

	if (new_exe_file)
		get_file(new_exe_file);
	rcu_assign_pointer(mm->exe_file, new_exe_file);
	if (old_exe_file)
		fput(old_exe_file);
	return 0;
}

/**
 * replace_mm_exe_file - replace a reference to the mm's executable file
 * @mm: The mm to change.
 * @new_exe_file: The new file to use.
 *
 * This changes mm's executable file (shown as symlink /proc/[pid]/exe).
 *
 * Main user is sys_prctl(PR_SET_MM_MAP/EXE_FILE).
 */
int replace_mm_exe_file(struct mm_struct *mm, struct file *new_exe_file)
{
	struct vm_area_struct *vma;
	struct file *old_exe_file;
	int ret = 0;

	/* Forbid mm->exe_file change if old file still mapped. */
	old_exe_file = get_mm_exe_file(mm);
	if (old_exe_file) {
		VMA_ITERATOR(vmi, mm, 0);
		mmap_read_lock(mm);
		for_each_vma(vmi, vma) {
			if (!vma->vm_file)
				continue;
			if (path_equal(&vma->vm_file->f_path,
				       &old_exe_file->f_path)) {
				ret = -EBUSY;
				break;
			}
		}
		mmap_read_unlock(mm);
		fput(old_exe_file);
		if (ret)
			return ret;
	}
				       &old_exe_file->f_path)) {
				ret = -EBUSY;
				break;
			}
		}
		mmap_read_unlock(mm);
		fput(old_exe_file);
		if (ret)
			return ret;
	}

	get_file(new_exe_file);

	/* set the new file */
	mmap_write_lock(mm);
	old_exe_file = rcu_dereference_raw(mm->exe_file);
	rcu_assign_pointer(mm->exe_file, new_exe_file);
	mmap_write_unlock(mm);

	if (old_exe_file)
		fput(old_exe_file);
	return 0;
}

/**
 * get_mm_exe_file - acquire a reference to the mm's executable file
 * @mm: The mm of interest.
 *
 * Returns %NULL if mm has no associated executable file.
 * User must release file via fput().
 */
struct file *get_mm_exe_file(struct mm_struct *mm)
{
	struct file *exe_file;

	rcu_read_lock();
	exe_file = get_file_rcu(&mm->exe_file);
	rcu_read_unlock();
	return exe_file;
}

/**
 * get_task_exe_file - acquire a reference to the task's executable file
 * @task: The task.
 *
 * Returns %NULL if task's mm (if any) has no associated executable file or
 * this is a kernel thread with borrowed mm (see the comment above get_task_mm).
 * User must release file via fput().
 */
struct file *get_task_exe_file(struct task_struct *task)
{
	struct file *exe_file = NULL;
	struct mm_struct *mm;

	task_lock(task);
	mm = task->mm;
	if (mm) {
		if (!(task->flags & PF_KTHREAD))
			exe_file = get_mm_exe_file(mm);
	}
	task_unlock(task);
	return exe_file;
}

/**
 * get_task_mm - acquire a reference to the task's mm
 * @task: The task.
 *
 * Returns %NULL if the task has no mm.  Checks PF_KTHREAD (meaning
 * this kernel workthread has transiently adopted a user mm with use_mm,
 * to do its AIO) is not set and if so returns a reference to it, after
 * bumping up the use count.  User must release the mm via mmput()
 * after use.  Typically used by /proc and ptrace.
 */
struct mm_struct *get_task_mm(struct task_struct *task)
{
	struct mm_struct *mm;

	task_lock(task);
	mm = task->mm;
	if (mm) {
		if (task->flags & PF_KTHREAD)
			mm = NULL;
		else
			mmget(mm);
	}
	task_unlock(task);
	return mm;
}
EXPORT_SYMBOL_GPL(get_task_mm);

struct mm_struct *mm_access(struct task_struct *task, unsigned int mode)
{
	struct mm_struct *mm;
	int err;

	err =  down_read_killable(&task->signal->exec_update_lock);
	if (err)
		return ERR_PTR(err);

	mm = get_task_mm(task);
	if (mm && mm != current->mm &&
			!ptrace_may_access(task, mode)) {
		mmput(mm);
		mm = ERR_PTR(-EACCES);
	}
	up_read(&task->signal->exec_update_lock);

	return mm;
}

static void complete_vfork_done(struct task_struct *tsk)
{
	struct completion *vfork;

	task_lock(tsk);
	vfork = tsk->vfork_done;
	if (likely(vfork)) {
		tsk->vfork_done = NULL;
		complete(vfork);
	}
	task_unlock(tsk);
}

static int wait_for_vfork_done(struct task_struct *child,
				struct completion *vfork)
{
	unsigned int state = TASK_KILLABLE|TASK_FREEZABLE;
	int killed;

	cgroup_enter_frozen();
	killed = wait_for_completion_state(vfork, state);
	cgroup_leave_frozen(false);

	if (killed) {
		task_lock(child);
		child->vfork_done = NULL;
		task_unlock(child);
	}

	put_task_struct(child);
	return killed;
}

/* Please note the differences between mmput and mm_release.
 * mmput is called whenever we stop holding onto a mm_struct,
 * error success whatever.
 *
 * mm_release is called after a mm_struct has been removed
 * from the current process.
 *
 * This difference is important for error handling, when we
 * only half set up a mm_struct for a new process and need to restore
 * the old one.  Because we mmput the new mm_struct before
 * restoring the old one. . .
 * Eric Biederman 10 January 1998
 */
static void mm_release(struct task_struct *tsk, struct mm_struct *mm)
{
	uprobe_free_utask(tsk);

	/* Get rid of any cached register state */
	deactivate_mm(tsk, mm);

	/*
	 * Signal userspace if we're not exiting with a core dump
	 * because we want to leave the value intact for debugging
	 * purposes.
	 */
	if (tsk->clear_child_tid) {
		if (atomic_read(&mm->mm_users) > 1) {
			/*
			 * We don't check the error code - if userspace has
			 * not set up a proper pointer then tough luck.
			 */
			put_user(0, tsk->clear_child_tid);
			do_futex(tsk->clear_child_tid, FUTEX_WAKE,
					1, NULL, NULL, 0, 0);
		}
				       &old_exe_file->f_path)) {
				ret = -EBUSY;
				break;
			}
		}
		mmap_read_unlock(mm);
		fput(old_exe_file);
		if (ret)
			return ret;
	}

	get_file(new_exe_file);

	/* set the new file */
	mmap_write_lock(mm);
	old_exe_file = rcu_dereference_raw(mm->exe_file);
	rcu_assign_pointer(mm->exe_file, new_exe_file);
	mmap_write_unlock(mm);

	if (old_exe_file)
		fput(old_exe_file);
	return 0;
}

/**
 * get_mm_exe_file - acquire a reference to the mm's executable file
 * @mm: The mm of interest.
 *
 * Returns %NULL if mm has no associated executable file.
 * User must release file via fput().
 */
struct file *get_mm_exe_file(struct mm_struct *mm)
{
	struct file *exe_file;

	rcu_read_lock();
	exe_file = get_file_rcu(&mm->exe_file);
	rcu_read_unlock();
	return exe_file;
}

/**
 * get_task_exe_file - acquire a reference to the task's executable file
 * @task: The task.
 *
 * Returns %NULL if task's mm (if any) has no associated executable file or
 * this is a kernel thread with borrowed mm (see the comment above get_task_mm).
 * User must release file via fput().
 */
struct file *get_task_exe_file(struct task_struct *task)
{
	struct file *exe_file = NULL;
	struct mm_struct *mm;

	task_lock(task);
	mm = task->mm;
	if (mm) {
		if (!(task->flags & PF_KTHREAD))
			exe_file = get_mm_exe_file(mm);
	}