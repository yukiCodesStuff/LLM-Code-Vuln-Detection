/*
 *  linux/fs/pipe.c
 *
 *  Copyright (C) 1991, 1992, 1999  Linus Torvalds
 */

#include <linux/mm.h>
#include <linux/file.h>
#include <linux/poll.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/log2.h>
#include <linux/mount.h>
#include <linux/magic.h>
#include <linux/pipe_fs_i.h>
#include <linux/uio.h>
#include <linux/highmem.h>
#include <linux/pagemap.h>
#include <linux/audit.h>
#include <linux/syscalls.h>
#include <linux/fcntl.h>

#include <asm/uaccess.h>
#include <asm/ioctls.h>

#include "internal.h"

/*
 * The max size that a non-root user is allowed to grow the pipe. Can
 * be set by root in /proc/sys/fs/pipe-max-size
 */
unsigned int pipe_max_size = 1048576;

/*
 * Minimum pipe size, as required by POSIX
 */
unsigned int pipe_min_size = PAGE_SIZE;

/* Maximum allocatable pages per user. Hard limit is unset by default, soft
 * matches default values.
 */
unsigned long pipe_user_pages_hard;
unsigned long pipe_user_pages_soft = PIPE_DEF_BUFFERS * INR_OPEN_CUR;

/*
 * We use a start+len construction, which provides full use of the 
 * allocated memory.
 * -- Florian Coosmann (FGC)
 * 
 * Reads with count = 0 should always return 0.
 * -- Julian Bradfield 1999-06-07.
 *
 * FIFOs and Pipes now generate SIGIO for both readers and writers.
 * -- Jeremy Elson <jelson@circlemud.org> 2001-08-16
 *
 * pipe_read & write cleanup
 * -- Manfred Spraul <manfred@colorfullife.com> 2002-05-09
 */

static void pipe_lock_nested(struct pipe_inode_info *pipe, int subclass)
{
	if (pipe->files)
		mutex_lock_nested(&pipe->mutex, subclass);
}
	if ((filp->f_mode & FMODE_WRITE) && retval >= 0) {
		retval = fasync_helper(fd, filp, on, &pipe->fasync_writers);
		if (retval < 0 && (filp->f_mode & FMODE_READ))
			/* this can happen only if on == T */
			fasync_helper(-1, filp, 0, &pipe->fasync_readers);
	}
	__pipe_unlock(pipe);
	return retval;
}

static void account_pipe_buffers(struct pipe_inode_info *pipe,
                                 unsigned long old, unsigned long new)
{
	atomic_long_add(new - old, &pipe->user->pipe_bufs);
}

static bool too_many_pipe_buffers_soft(struct user_struct *user)
{
	return pipe_user_pages_soft &&
	       atomic_long_read(&user->pipe_bufs) >= pipe_user_pages_soft;
}

static bool too_many_pipe_buffers_hard(struct user_struct *user)
{
	return pipe_user_pages_hard &&
	       atomic_long_read(&user->pipe_bufs) >= pipe_user_pages_hard;
}

struct pipe_inode_info *alloc_pipe_info(void)
{
	struct pipe_inode_info *pipe;

	pipe = kzalloc(sizeof(struct pipe_inode_info), GFP_KERNEL);
	if (pipe) {
		unsigned long pipe_bufs = PIPE_DEF_BUFFERS;
		struct user_struct *user = get_current_user();

		if (!too_many_pipe_buffers_hard(user)) {
			if (too_many_pipe_buffers_soft(user))
				pipe_bufs = 1;
			pipe->bufs = kzalloc(sizeof(struct pipe_buffer) * pipe_bufs, GFP_KERNEL);
		}

		if (pipe->bufs) {
			init_waitqueue_head(&pipe->wait);
			pipe->r_counter = pipe->w_counter = 1;
			pipe->buffers = pipe_bufs;
			pipe->user = user;
			account_pipe_buffers(pipe, 0, pipe_bufs);
			mutex_init(&pipe->mutex);
			return pipe;
		}
		free_uid(user);
		kfree(pipe);
	}
{
	int i;

	account_pipe_buffers(pipe, pipe->buffers, 0);
	free_uid(pipe->user);
	for (i = 0; i < pipe->buffers; i++) {
		struct pipe_buffer *buf = pipe->bufs + i;
		if (buf->ops)
			buf->ops->release(pipe, buf);
	}
	if (pipe->nrbufs) {
		unsigned int tail;
		unsigned int head;

		tail = pipe->curbuf + pipe->nrbufs;
		if (tail < pipe->buffers)
			tail = 0;
		else
			tail &= (pipe->buffers - 1);

		head = pipe->nrbufs - tail;
		if (head)
			memcpy(bufs, pipe->bufs + pipe->curbuf, head * sizeof(struct pipe_buffer));
		if (tail)
			memcpy(bufs + head, pipe->bufs, tail * sizeof(struct pipe_buffer));
	}

	account_pipe_buffers(pipe, pipe->buffers, nr_pages);
	pipe->curbuf = 0;
	kfree(pipe->bufs);
	pipe->bufs = bufs;
	pipe->buffers = nr_pages;
	return nr_pages * PAGE_SIZE;
}

/*
 * Currently we rely on the pipe array holding a power-of-2 number
 * of pages.
 */
static inline unsigned int round_pipe_size(unsigned int size)
{
	unsigned long nr_pages;

	nr_pages = (size + PAGE_SIZE - 1) >> PAGE_SHIFT;
	return roundup_pow_of_two(nr_pages) << PAGE_SHIFT;
}

/*
 * This should work even if CONFIG_PROC_FS isn't set, as proc_dointvec_minmax
 * will return an error.
 */
int pipe_proc_fn(struct ctl_table *table, int write, void __user *buf,
		 size_t *lenp, loff_t *ppos)
{
	int ret;

	ret = proc_dointvec_minmax(table, write, buf, lenp, ppos);
	if (ret < 0 || !write)
		return ret;

	pipe_max_size = round_pipe_size(pipe_max_size);
	return ret;
}

/*
 * After the inode slimming patch, i_pipe/i_bdev/i_cdev share the same
 * location, so checking ->i_pipe is not enough to verify that this is a
 * pipe.
 */
struct pipe_inode_info *get_pipe_info(struct file *file)
{
	return file->f_op == &pipefifo_fops ? file->private_data : NULL;
}

long pipe_fcntl(struct file *file, unsigned int cmd, unsigned long arg)
{
	struct pipe_inode_info *pipe;
	long ret;

	pipe = get_pipe_info(file);
	if (!pipe)
		return -EBADF;

	__pipe_lock(pipe);

	switch (cmd) {
	case F_SETPIPE_SZ: {
		unsigned int size, nr_pages;

		size = round_pipe_size(arg);
		nr_pages = size >> PAGE_SHIFT;

		ret = -EINVAL;
		if (!nr_pages)
			goto out;

		if (!capable(CAP_SYS_RESOURCE) && size > pipe_max_size) {
			ret = -EPERM;
			goto out;
		} else if ((too_many_pipe_buffers_hard(pipe->user) ||
			    too_many_pipe_buffers_soft(pipe->user)) &&
		           !capable(CAP_SYS_RESOURCE) && !capable(CAP_SYS_ADMIN)) {
			ret = -EPERM;
			goto out;
		}
		ret = pipe_set_size(pipe, nr_pages);
		break;
		}
	case F_GETPIPE_SZ:
		ret = pipe->buffers * PAGE_SIZE;
		break;
	default:
		ret = -EINVAL;
		break;
	}
		if (!capable(CAP_SYS_RESOURCE) && size > pipe_max_size) {
			ret = -EPERM;
			goto out;
		} else if ((too_many_pipe_buffers_hard(pipe->user) ||
			    too_many_pipe_buffers_soft(pipe->user)) &&
		           !capable(CAP_SYS_RESOURCE) && !capable(CAP_SYS_ADMIN)) {
			ret = -EPERM;
			goto out;
		}