 */
unsigned int pipe_min_size = PAGE_SIZE;

/*
 * We use a start+len construction, which provides full use of the 
 * allocated memory.
 * -- Florian Coosmann (FGC)
	return retval;
}

struct pipe_inode_info *alloc_pipe_info(void)
{
	struct pipe_inode_info *pipe;

	pipe = kzalloc(sizeof(struct pipe_inode_info), GFP_KERNEL);
	if (pipe) {
		pipe->bufs = kzalloc(sizeof(struct pipe_buffer) * PIPE_DEF_BUFFERS, GFP_KERNEL);
		if (pipe->bufs) {
			init_waitqueue_head(&pipe->wait);
			pipe->r_counter = pipe->w_counter = 1;
			pipe->buffers = PIPE_DEF_BUFFERS;
			mutex_init(&pipe->mutex);
			return pipe;
		}
		kfree(pipe);
	}

	return NULL;
{
	int i;

	for (i = 0; i < pipe->buffers; i++) {
		struct pipe_buffer *buf = pipe->bufs + i;
		if (buf->ops)
			buf->ops->release(pipe, buf);
			memcpy(bufs + head, pipe->bufs, tail * sizeof(struct pipe_buffer));
	}

	pipe->curbuf = 0;
	kfree(pipe->bufs);
	pipe->bufs = bufs;
	pipe->buffers = nr_pages;
		if (!capable(CAP_SYS_RESOURCE) && size > pipe_max_size) {
			ret = -EPERM;
			goto out;
		}
		ret = pipe_set_size(pipe, nr_pages);
		break;
		}