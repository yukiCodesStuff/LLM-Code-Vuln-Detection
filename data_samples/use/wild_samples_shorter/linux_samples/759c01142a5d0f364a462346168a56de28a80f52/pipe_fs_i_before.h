 *	@fasync_readers: reader side fasync
 *	@fasync_writers: writer side fasync
 *	@bufs: the circular array of pipe buffers
 **/
struct pipe_inode_info {
	struct mutex mutex;
	wait_queue_head_t wait;
	struct fasync_struct *fasync_readers;
	struct fasync_struct *fasync_writers;
	struct pipe_buffer *bufs;
};

/*
 * Note on the nesting of these functions:
void pipe_double_lock(struct pipe_inode_info *, struct pipe_inode_info *);

extern unsigned int pipe_max_size, pipe_min_size;
int pipe_proc_fn(struct ctl_table *, int, void __user *, size_t *, loff_t *);


/* Drop the inode semaphore and wait for a pipe event, atomically */