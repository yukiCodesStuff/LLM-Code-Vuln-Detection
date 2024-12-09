unsigned int unix_tot_inflight;


static struct sock *unix_get_socket(struct file *filp)
{
	struct sock *u_sock = NULL;
	struct inode *inode = filp->f_path.dentry->d_inode;

}

static bool gc_in_progress = false;

void wait_for_unix_gc(void)
{
	wait_event(unix_gc_wait, gc_in_progress == false);
}

/* The external entry point: unix_gc() */