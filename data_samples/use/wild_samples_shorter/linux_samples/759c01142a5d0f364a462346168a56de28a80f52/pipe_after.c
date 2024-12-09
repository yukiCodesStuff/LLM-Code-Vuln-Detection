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

	return NULL;
{
	int i;

	account_pipe_buffers(pipe, pipe->buffers, 0);
	free_uid(pipe->user);
	for (i = 0; i < pipe->buffers; i++) {
		struct pipe_buffer *buf = pipe->bufs + i;
		if (buf->ops)
			buf->ops->release(pipe, buf);
			memcpy(bufs + head, pipe->bufs, tail * sizeof(struct pipe_buffer));
	}

	account_pipe_buffers(pipe, pipe->buffers, nr_pages);
	pipe->curbuf = 0;
	kfree(pipe->bufs);
	pipe->bufs = bufs;
	pipe->buffers = nr_pages;
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