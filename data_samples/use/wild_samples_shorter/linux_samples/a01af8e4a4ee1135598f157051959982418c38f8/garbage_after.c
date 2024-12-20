unsigned int unix_tot_inflight;


struct sock *unix_get_socket(struct file *filp)
{
	struct sock *u_sock = NULL;
	struct inode *inode = filp->f_path.dentry->d_inode;

}

static bool gc_in_progress = false;
#define UNIX_INFLIGHT_TRIGGER_GC 16000

void wait_for_unix_gc(void)
{
	/*
	 * If number of inflight sockets is insane,
	 * force a garbage collect right now.
	 */
	if (unix_tot_inflight > UNIX_INFLIGHT_TRIGGER_GC && !gc_in_progress)
		unix_gc();
	wait_event(unix_gc_wait, gc_in_progress == false);
}

/* The external entry point: unix_gc() */